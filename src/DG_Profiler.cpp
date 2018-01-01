#include "DG_Profiler.h"

#include <map>
#include <stack>

#include "DG_Windows.h"

namespace DG
{
	static DebugProfiler profiler;

	DebugEvent DebugProfiler::GetNextEvent()
	{
		SDL_LockMutex(_mutex);
		DebugEvent event = _queue.front();
		_queue.pop();
		SDL_UnlockMutex(_mutex);

		return event;
	}

	DebugProfiler* GetProfiler()
	{
		return &profiler;
	}


	int ProfilerWork(void* data, FILE* output_stream)
	{
		// Happens once at startup
		SDL_sem** sem = (SDL_sem**)data;
		if (*sem == nullptr)
		{
			*sem = profiler._semaphore;
			return 0;
		}

		if (profiler._queue.empty())
			return 1;

		DebugEvent event = profiler.GetNextEvent();

		if (profiler._stopRecording)
			return 0;

		// Handle Frame Start
		static DebugEvent frameStartEvent(0, 0, "", Error);

		static bool isFirst = true;
#if 0
		fprintf(output_stream,
			"%s{\"name\": \"%s\", \"ph\": \"%s\", \"pid\": 0, \"tid\": %i, \"ts\": %f}\n",
			isFirst ? "" : ",",
			event.Name.c_str(),
			event.Type == FrameStart || event.Type == Start ? "B" : "E",
			event.ThreadId,
			event.Clock * 1000000 / profiler._processor_frequency);
#endif

		isFirst = false;
		if (event.Type == FrameStart)
		{
			Assert(!profiler._hasFrameStart);
			profiler._hasFrameStart = true;
			frameStartEvent = event;

			profiler._concatenatedResultMap.clear();
			profiler._resultMap.clear();
			for (auto &pair : profiler._builderStacks)
			{
				while (!pair.second.empty())
					pair.second.pop();
			}
		}
		// Handle Frame End
		else if (profiler._hasFrameStart)
		{
			if (event.Type == FrameEnd)
			{
				profiler._hasFrameStart = false;

				{
					for (auto &pair : profiler._builderStacks)
					{
						Assert(pair.second.empty());
					}
				}

				// Build DebugFrame
				DebugFrame *currentFrame = &profiler._frames[profiler._currentIndex];
				currentFrame->TotalCycles = event.Clock - frameStartEvent.Clock;

				u32 previousFrameIndex = (profiler._currentIndex - 1) % DebugProfiler::FramesRecorded;
				currentFrame->FrameId = profiler._frames[previousFrameIndex].FrameId + 1;

				// Copy Data over and reset locally
				currentFrame->AveragedEvents = profiler._concatenatedResultMap;
				currentFrame->Events = profiler._resultMap;

				currentFrame->StartCycle = frameStartEvent.Clock;

				// ToDo: Just clear the inner vector not the whole map
				// This would avoid unnecessary memory allocations (since the event size per frame will stay realtively constant)
				profiler._concatenatedResultMap.clear();
				profiler._resultMap.clear();

				profiler._lastCompleteFrame = profiler._currentIndex;
				profiler._currentIndex = (profiler._currentIndex + 1) % DebugProfiler::FramesRecorded;

				// ToDo: Save profiler data to disk
			}
			else if (profiler._hasFrameStart)
			{
				// Check if Thread already has a Stack
				auto &stack = profiler._builderStacks[event.ThreadId];

				if (event.Type == Start)
				{
					stack.push(event);
				}
				else
				{
					Assert(stack.size() > 0);
					DebugEvent start = stack.top();
					stack.pop();

					// Update the average Event
					auto &eventResult = profiler._concatenatedResultMap[event.ThreadId][start.GUID];

					u64 usedCycles = event.Clock - start.Clock;
					eventResult.TotalElapsedCycles += usedCycles;
					eventResult.MaxElapsedCycles = std::max(usedCycles, eventResult.MaxElapsedCycles);

					eventResult.Name = start.Name;
					eventResult.FileName = start.FileName;
					eventResult.Counter = start.Counter;
					eventResult.LineNumber = start.LineNumber;

					// Create new evaluated event
					EvaluatedDebugEvent result;
					result.StartClock = start.Clock;
					result.ElapsedCycles = usedCycles;
					result.Name = start.Name;
					result.FileName = start.FileName;
					result.Counter = start.Counter;
					result.LineNumber = start.LineNumber;
					result.Level = (u32)stack.size();

					profiler._resultMap[event.ThreadId].push_back(result);
				}
			}
		}

		return 0;
	}

}
