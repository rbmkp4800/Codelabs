#include "XTest.Manager.Core.Storage.h"
#include "XTest.Manager.h"

using namespace XLib;
using namespace XTest::Manager::_Core;
using namespace XTest::Manager::_Core::_Storage;
using namespace XTest::Manager::Internal;

// TODO: deal with constructors of Solution and Problem objects
//		where they need to be ran in PoolAllocator or here

#define core getCore()

bool Storage::startup()
{
	diskWorkerThread.create(DiskWorkerThreadMain, this);
}

void Storage::createSolutionAsync(const char* source, uint32 sourceLength,
	XTLanguage language, XTProblemId problemId, XTTestingPolicy testingPolicy)
{
	Problem *problem = problemCache.find(problemId);

	Solution *solution = solutionAllocator.allocate();
	solution->problemId = problemId;
	solution->language = language;
	solution->testingPolicy = testingPolicy;
	solution->state = XTSolutionState::Waiting;
	solution->source = source;
	solution->sourceLength = sourceLength;
	solution->result = { 0 };

	if (problem)
	{
		solutionCache.insert(solution);
		core.onStorageSolutionCreationComplete(XTMSubmitSolutionResult::Success, solution);
	}
	else
	{
		DiskWorkerQueueItem item;
		item.type = DiskWorkerQueueItem::Type::CheckProblemFileAndCompleteSolutionCreation;
		item.checkProblemFileAndCompleteSolutionCreation.solution = solution;
		diskWorkerQueue.enqueue(item);
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
			case DiskWorkerQueueItem::Type::CheckProblemFileAndCompleteSolutionCreation:
			{
				auto& args = item.checkProblemFileAndCompleteSolutionCreation;
				XTProblemId problemId = args.solution->getProblemId();

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
					core.onStorageSolutionCreationComplete(XTMSubmitSolutionResult::Success, args.solution);
				}
				else
				{
					solutionAllocator.release(args.solution);
					core.onStorageSolutionCreationComplete(XTMSubmitSolutionResult::SystemError, args.solution);
				}

				break;
			}

		default:
			Debug::Crash(DbgMsgFmt("invalid DiskWorkerQueueItem::Type"));
		}
	}

	core.onStorageShutdownComplete();
}