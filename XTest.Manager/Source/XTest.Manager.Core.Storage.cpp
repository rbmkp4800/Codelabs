#include "XTest.Manager.Core.Storage.h"
#include "XTest.Manager.h"

using namespace XLib;
using namespace XTest::Manager::_Core;
using namespace XTest::Manager::_Core::_Storage;
using namespace XTest::Manager::Internal;

#define core getCore()

bool Storage::startup()
{
	diskWorkerThread.create(DiskWorkerThreadMain, this);
}

void Storage::createSolution(const char* source, uint32 sourceLength,
	XTLanguage language, XTProblemId problemId, XTTestingPolicy testingPolicy)
{
	Problem *problem = problemCache.find(problemId);

	auto handle = solutionCache.preAllocate();
	Solution &solution = handle.get();
	solution.problemId = problemId;
	solution.language = language;
	solution.testingPolicy = testingPolicy;
	solution.state = XTSolutionState::Waiting;
	solution.source = source;
	solution.sourceLength = sourceLength;

	if (problem)
	{
		solutionCache.insertPreAllocated(handle);

		core.onSolutionCreationComplete(XTMSubmitSolutionResult::Success, &solution);
	}
	else
	{
		DiskWorkerQueueItem item;
		item.checkProblemFileAndCompleteSolutionCreation.preAllocatedSolution = 
	}
}

uint32 __stdcall Storage::DiskWorkerThreadMain(Storage* self)
{
	self->diskWorkerThreadMain();
	return 0;
}

void Storage::diskWorkerThreadMain()
{
	// initialization
	{
		bool result = solutionFiles.open();
		core.onStorageStartupComplete(result);

		if (!result)
			return;
	}

	for (;;)
	{
		DiskWorkerQueueItem item = diskWorkerQueue.dequeue();
		switch (item.type)
		{
			case DiskWorkerQueueItemType::CheckProblemFileAndCompleteSolutionCreation:
			{
				auto args = item.checkProblemFileAndCompleteSolutionCreation;
				Solution &solution = args.preAllocatedSolutionHandle.get();
				XTProblemId problemId = solution.getProblemId();

				bool result = false;
				for each (ProblemFile& file in problemFiles)
				{
					if (!file.isOpened())
					{
						result = file.open(problemId);
						break;
					}
				}

				if (result)
				{
					solutionCache.insertPreAllocated(args.preAllocatedSolutionHandle);
					core.onSolutionCreationComplete(XTMSubmitSolutionResult::Success, &solution);
				}
				else
				{
					//solutionCache.releasePreAllocated();
					core.onSolutionCreationComplete(XTMSubmitSolutionResult::SystemError, &solution);
				}

				break;
			}

		default:
			break;
		}
	}

	core.onStorageShutdownComplete();
}