#pragma once
#include "DG_Include.h"
#include <unordered_set>

namespace DG
{
	extern bool g_JobQueueShutdownRequested;
	
	struct Job;
	typedef void(*JobFunction)(Job*, const void *);

	struct Job
	{
		JobFunction function;
		Job *parent;
		SDL_atomic_t unfinishedJobs{ -1 }; // 1 job can be run, >1 amount of children to finish, -1 job finished, 0 job finishing
		char data[44]; // Padded to be 64 byte in size!
	};
	static_assert((sizeof(Job::function) + sizeof(Job::parent) + sizeof(Job::unfinishedJobs) + sizeof(Job::data)) == 64, "sizeof(Job) needs to be a multiple of 64Bytes");

	class JobSystem
	{
	public:
		static Job *CreateJob(JobFunction function);
		static Job *CreateJobAsChild(Job* parent, JobFunction function);

		static void Wait(const Job* job);
		static void Run(Job *job);

		static void CreateAndRegisterWorker();
		static void RegisterWorker();
		


		class JobWorkQueue
		{
			friend class JobSystem;
		public:
			JobWorkQueue(Job** queue, u32 size);


		private:
			Job* TryGetLocalJob();
			Job* Steal();
			void Finish(Job* job);

			Job** _queue;
			u32 _size;
			s32 _bottom = 0;
			s32 _top = 0;

		};

	private:
		static std::unordered_set<JobWorkQueue *> _threadTLSIds;
		static void TryDoJob();
		static int JobQueueWorkerFunction(void *data);
		
	};
}
