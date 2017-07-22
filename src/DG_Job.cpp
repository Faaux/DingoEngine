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

	std::unordered_set<JobSystem::JobWorkQueue *> JobSystem::_threadTLSIds;
	static SDL_mutex* _mutex = SDL_CreateMutex();

	JobSystem::JobWorkQueue::JobWorkQueue(Job** queue, u32 size)
		: _queue(queue), _size(size)
	{
	}

	Job* JobSystem::JobWorkQueue::TryGetLocalJob()
	{
		s32 top = _top - 1;
		_top = top;

		MemoryBarrier();

		s32 bottom = _bottom.value;

		if (bottom <= top)
		{
			Job* job = LocalJobQueue[top & JOB_MASK];
			if (bottom != top)
			{
				return job;
			}
			if (!SDL_AtomicCAS(&_bottom, bottom, bottom + 1))
			{
				job = nullptr;
			}
			_top = bottom + 1;
			return job;
		}
		_top = bottom;
		return nullptr;

	}

	Job* JobSystem::JobWorkQueue::Steal()
	{
		u32 bottom = _bottom.value;

		SDL_CompilerBarrier();
		u32 top = _top;

		if (bottom < top)
		{
			Job* job = _queue[bottom & JOB_MASK];

			// ToDo: Make this work on all platforms this is windows specific! (SDL forces their own atomic struct which is annoying)
			if (SDL_AtomicCAS(&_bottom, bottom, bottom + 1))
			{
				return job;
			}
		}
		return nullptr;
	}

	void JobSystem::JobWorkQueue::Finish(Job* job)
	{
		const s32 unfinishedJobs = SDL_AtomicAdd(&job->unfinishedJobs, -1);
		if (unfinishedJobs == 1)
		{
			if (job->parent)
			{
				Finish(job->parent);
			}

			SDL_AtomicAdd(&job->unfinishedJobs, -1);
			Assert(job->unfinishedJobs.value == -1);
		}
	}

	Job* JobSystem::CreateJob(JobFunction function)
	{
		u32 index = LocalJobBufferIndex & JOB_MASK;
		++LocalJobBufferIndex;
		Job& job = LocalJobBuffer[index];
		Assert(job.unfinishedJobs.value == -1);
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
		Assert(job.unfinishedJobs.value == -1);
		job.function = function;
		job.parent = parent;
		SDL_AtomicSet(&job.unfinishedJobs, 1);
		SDL_memset(&job.data, 0, ArrayCount(job.data));

		return &job;
	}

	void JobSystem::Wait(const Job* job)
	{
		// wait until the job has completed. in the meantime, work on any other job.
		while (job->unfinishedJobs.value != -1)
		{
			TryDoJob();
		}
	}

	void JobSystem::Run(Job* job)
	{
		LocalJobQueue[LocalQueue._top & JOB_MASK] = job;

		SDL_CompilerBarrier();
		++LocalQueue._top;
	}

	void JobSystem::CreateAndRegisterWorker()
	{
		SDL_CreateThread(JobQueueWorkerFunction, "Worker", nullptr);
	}

	void JobSystem::RegisterWorker()
	{
		SDL_LockMutex(_mutex);
		_threadTLSIds.insert(&LocalQueue);
		SDL_UnlockMutex(_mutex);
	}

	void JobSystem::TryDoJob()
	{
		Job* job = LocalQueue.TryGetLocalJob();
		if (!job)
		{
			for (JobWorkQueue * queue : _threadTLSIds)
			{
				if (!queue || queue == &LocalQueue)
					continue;

				job = queue->Steal();

				if (job)
					break;
			}
			if (!job)
				return;
		}
		Assert(job->unfinishedJobs.value != -1);
		// Execute Job
		job->function(job, job->data);
		LocalQueue.Finish(job);
	}

	int JobSystem::JobQueueWorkerFunction(void* data)
	{
		RegisterWorker();

		while (!g_JobQueueShutdownRequested)
		{
			TryDoJob();
		}
		return 0;
	}
}
