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
		SDL_atomic_t unfinishedJobs{ 0 };
		char data[44]; // Padded to be 64 byte in size!
	};
	static_assert((sizeof(Job::function) + sizeof(Job::parent) + sizeof(Job::unfinishedJobs) + sizeof(Job::data)) == 64, "sizeof(Job) needs to be a multiple of 64Bytes");

	class JobSystem
	{
	public:
		static Job *CreateJob(JobFunction function);
		static Job *CreateJobAsChild(Job* parent, JobFunction function);

		static void Wait(Job* job);
		static void Run(Job *job);
		static void Finish(Job* job);

		static void CreateAndRegisterWorker();
		static bool RegisterWorker();
		
		class JobWorkQueue
		{
			friend class JobSystem;
		public:
			JobWorkQueue(Job** queue, u32 size);
			Job* GetJob();

		private:
			void Push(Job *job);
			Job* Pop();
			Job* Steal();
			

			Job** _jobs;
			u32 _size;
			SDL_atomic_t _bottom;
			s32 _top = 0;

		};

	private:
		static int JobQueueWorkerFunction(void *data);
	};
}
