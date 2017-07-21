#pragma once
#include "DG_Include.h"
#include <unordered_set>

namespace DG
{
	extern bool g_JobQueueShutdownRequested;
	int JobQueueWorkerFunction(void *data);

	struct Job;
	typedef void(*JobFunction)(Job*, const void *);

	struct Job
	{
		JobFunction function;
		Job *parent;
		SDL_atomic_t unfinishedJobs; // 1 job can be run, >1 amount of children to finish, -1 job finished, 0 job finishing
		char data[44]; // Padded to be 64 byte in size!
	};
	static_assert((sizeof(Job::function) + sizeof(Job::parent) + sizeof(Job::unfinishedJobs) + sizeof(Job::data)) == 64, "sizeof(Job) needs to be a multiple of 64Bytes");

	class JobWorkQueue
	{
		friend bool RunWork();
		friend int JobQueueWorkerFunction(void *data);
	public:
		JobWorkQueue(Job** queue, u32 size);
		
		
	private:
		Job *CreateJob(JobFunction function);
		Job* GetJob();
		void Run(Job *job);
		void Finish(Job* job);

		Job** _queue;
		u32 _size;
		u32 topIndex = 0;
		u32 bottomIndex = 0;
	};

	class JobSystem
	{
	public:
		static void CreateAndRegisterWorker();
		static void RegisterWorker();
	private:
		static std::unordered_set<SDL_TLSID> _threadTLSIds;
	};
}
