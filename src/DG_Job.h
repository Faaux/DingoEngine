#pragma once
#include <unordered_set>
#include "DG_Include.h"

namespace DG
{
extern bool g_JobQueueShutdownRequested;

struct Job;
typedef void (*JobFunction)(Job*, const void*);

struct Job
{
    JobFunction function;
    Job* parent;
    SDL_atomic_t unfinishedJobs{0};
    char data[12];  // Padded to be 32 byte in size!
    bool CheckIsDone();
};
static_assert((sizeof(Job::function) + sizeof(Job::parent) + sizeof(Job::unfinishedJobs) +
               sizeof(Job::data)) == 32,
              "sizeof(Job) needs to be a multiple of 32Bytes");

class JobSystem
{
   public:
    static Job* CreateJob(JobFunction function);
    static Job* CreateJobAsChild(Job* parent, JobFunction function);

    static void Wait(Job* job);
    static void Run(Job* job);
    static void Finish(Job* job);

    static void CreateAndRegisterWorker();
    static bool RegisterWorker();

    static void RunCurrentThreadAsWorker();

    class JobWorkQueue
    {
        friend class JobSystem;

       public:
        JobWorkQueue(Job** queue, u32 size);
        Job* GetJob();

       private:
        void Push(Job* job);
        Job* Pop();
        Job* Steal();

        Job** _jobs;
        u32 _size;
        SDL_atomic_t _bottom;
        s32 _top = 0;
    };

   private:
    static void RunWorker();
    static int JobQueueWorkerFunction(void* data);
};
}  // namespace DG
