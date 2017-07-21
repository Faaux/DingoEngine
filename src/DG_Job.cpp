#include "DG_Job.h"
#include "SDL/SDL.h"
#include <atomic>

namespace DG
{
	thread_local u32 JobBufferIndex = 0;
	thread_local Job JobBuffer[4096];
	thread_local Job* JobQueue[4096];
	thread_local JobWorkQueue Queue(JobQueue, ArrayCount(JobQueue));

	bool g_JobQueueShutdownRequested = false;

	std::unordered_set<SDL_TLSID> JobSystem::_threadTLSIds;
	static SDL_mutex* _mutex = SDL_CreateMutex();

	// Returns false if no work was done
	static bool RunWork()
	{
		Job* job = Queue.GetJob();
		if (job)
		{
			// Execute Job
			job->function(job, job->data);
			Queue.Finish(job);
			return true;
		}
		return false;
	}	

	int JobQueueWorkerFunction(void *data)
	{
		JobSystem::RegisterWorker();

		while (!g_JobQueueShutdownRequested)
		{
			if(!RunWork())
			{
				// No work left for this thread, try to steal from another thread
				SDL_Delay(0);
			}
		}
		return 0;
	}

	JobWorkQueue::JobWorkQueue(Job** queue, u32 size)
		: _queue(queue), _size(size)
	{
	}

	Job* JobWorkQueue::CreateJob(JobFunction function)
	{
		Job& job = JobBuffer[JobBufferIndex++];
		job.function = function;
		job.parent = nullptr;
		SDL_AtomicSet(&job.unfinishedJobs, 1);
		// ToDo: Zero Data
		
		return &job;
	}

	Job* JobWorkQueue::GetJob()
	{
		if (bottomIndex > 0)
		{
			--bottomIndex;
			return JobQueue[bottomIndex % ArrayCount(JobQueue)];
		}
			
		return nullptr;
	}

	void JobWorkQueue::Run(Job* job)
	{
		const u32 index = bottomIndex++;
		JobQueue[index % ArrayCount(JobQueue)] = job;
	}

	void JobWorkQueue::Finish(Job* job)
	{
		const s32 unfinishedJobs = SDL_AtomicAdd(&job->unfinishedJobs, -1);
		if(unfinishedJobs == 1)
		{
			if(job->parent)
			{
				Finish(job->parent);
			}

			SDL_AtomicAdd(&job->unfinishedJobs, -1);
		}
	}

	void JobSystem::CreateAndRegisterWorker()
	{
		SDL_CreateThread(JobQueueWorkerFunction, "Worker", nullptr);
	}
	
	void JobSystem::RegisterWorker()
	{
		SDL_LockMutex(_mutex);
		SDL_TLSID id = SDL_TLSCreate();
		SDL_TLSSet(id, &Queue, 0);
		_threadTLSIds.insert(id);
		SDL_UnlockMutex(_mutex);
	}
}
