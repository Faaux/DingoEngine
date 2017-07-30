#include "DG_Job.h"
#include <SDL.h>
#include <atomic>
#include "DG_Windows.h"

namespace DG
{
	static const u32 JOB_COUNT = 4096;
	static const u32 JOB_MASK = JOB_COUNT - 1u;

	thread_local u32 LocalJobBufferIndex = 0;
	thread_local Job LocalJobBuffer[JOB_COUNT];
	thread_local Job* LocalJobQueue[JOB_COUNT];
	thread_local JobSystem::JobWorkQueue LocalQueue(LocalJobQueue, JOB_COUNT);

	bool g_JobQueueShutdownRequested = false;

	static JobSystem::JobWorkQueue* _queues[64];
	static SDL_mutex* _mutex = SDL_CreateMutex();
	static SDL_sem* _sem = SDL_CreateSemaphore(0);
	static s32 _workerCount = 0;
	JobSystem::JobWorkQueue::JobWorkQueue(Job** queue, u32 size)
		: _jobs(queue), _size(size)
	{
	}

	Job* JobSystem::JobWorkQueue::GetJob()
	{
		Job* job = Pop();
		if (!job)
		{
			// Steal Job
			u32 index = rand() % _workerCount;
			JobWorkQueue* queueToStealFrom = _queues[index];
			if (&LocalQueue != queueToStealFrom)
			{
				job = queueToStealFrom->Steal();
			}
		}
		return job;
	}

	void JobSystem::JobWorkQueue::Push(Job* job)
	{
		s32 t = _top;
		_jobs[t & JOB_MASK] = job;

		SDL_CompilerBarrier();

		_top = t + 1;
	}

	Job* JobSystem::JobWorkQueue::Pop()
	{
		s32 t = _top - 1;
		_top = t;

		MemoryBarrier(); // Can't find a SDL equivalent

		s32 b = SDL_AtomicGet(&_bottom);

		// Empty queue
		if (b > t)
		{
			_top = b;
			return nullptr;
		}

		// not empty
		Job* job = _jobs[t & JOB_MASK];

		// Not Last Item
		if (b != t)
			return job;

		// Check if a steal operation got there before us
		if (!SDL_AtomicCAS(&_bottom, b, b + 1))
			job = nullptr; // Someone stole the job

		_top = b + 1;
		return job;
	}

	Job* JobSystem::JobWorkQueue::Steal()
	{
		long b = SDL_AtomicGet(&_bottom);

		SDL_CompilerBarrier();

		long t = _top;
		if (b < t)
		{
			// non-empty queue
			Job* job = _jobs[b & JOB_MASK];
			if (SDL_AtomicCAS(&_bottom, b, b + 1))
			{
				return job;
			}
		}
		return nullptr;
	}

	void JobSystem::Finish(Job* job)
	{
		const s32 unfinishedJobs = SDL_AtomicAdd(&job->unfinishedJobs, -1);
		if (unfinishedJobs == 1)
		{
			if (job->parent)
			{
				Finish(job->parent);
			}
		}
	}

	Job* JobSystem::CreateJob(JobFunction function)
	{
		u32 index = LocalJobBufferIndex & JOB_MASK;
		++LocalJobBufferIndex;
		Job& job = LocalJobBuffer[index];
		Assert(SDL_AtomicGet(&job.unfinishedJobs) == 0);
		job.function = function;
		job.parent = nullptr;
		SDL_AtomicSet(&job.unfinishedJobs, 1);
		SDL_memset(&job.data, 0, ArrayCount(job.data));

		return &job;
	}

	Job* JobSystem::CreateJobAsChild(Job* parent, JobFunction function)
	{
		SDL_AtomicAdd(&parent->unfinishedJobs, 1);

		u32 index = LocalJobBufferIndex & JOB_MASK;
		++LocalJobBufferIndex;
		Job& job = LocalJobBuffer[index];

		Assert(SDL_AtomicGet(&job.unfinishedJobs) == 0);

		job.function = function;
		job.parent = parent;
		SDL_AtomicSet(&job.unfinishedJobs, 1);
		SDL_memset(&job.data, 0, ArrayCount(job.data));

		return &job;
	}

	void JobSystem::Wait(Job* job)
	{
		// wait until the job has completed. in the meantime, work on any other job.
		while (SDL_AtomicGet(&job->unfinishedJobs) != 0)
		{
			if (SDL_SemTryWait(_sem) == 0)
			{
				Job* jobToBeDone = nullptr;
				while (!jobToBeDone)
				{
					jobToBeDone = LocalQueue.GetJob();
				}

				jobToBeDone->function(jobToBeDone, jobToBeDone->data);
				Finish(jobToBeDone);
			}
		}
	}

	void JobSystem::Run(Job* job)
	{
		LocalQueue.Push(job);
		SDL_SemPost(_sem);
	}

	void JobSystem::CreateAndRegisterWorker()
	{
		SDL_CreateThread(JobQueueWorkerFunction, "Worker", nullptr);
	}

	bool JobSystem::RegisterWorker()
	{
		SDL_LockMutex(_mutex);
		
		if (_workerCount >= ArrayCount(_queues))
		{
			Assert(false);
			return false;
		}
		_queues[_workerCount++] = &LocalQueue;
		SDL_UnlockMutex(_mutex);
		return true;
	}

	int JobSystem::JobQueueWorkerFunction(void* data)
	{
		if (!RegisterWorker())
			return -1;
		while (!g_JobQueueShutdownRequested)
		{
			SDL_SemWait(_sem);
			// We got the semaphore, meaning we need to do one job!
			Job* job = nullptr;
			while (!job)
			{
				job = LocalQueue.GetJob();
			}

			job->function(job, job->data);
			Finish(job);
		}
		return 0;
	}
}
