#include <stdio.h>							// for config file loading
#include <XLib.System.Network.Socket.h>		//   ---//---

#include "XTest.Manager.Core.Storage.h"
#include "XTest.Manager.h"

using namespace XLib;
using namespace XTest::Manager::_Core;
using namespace XTest::Manager::_Core::_Storage;
using namespace XTest::Manager::Internal;

// TODO: deal with constructors of Solution and Problem objects
//		where they need to be ran in PoolAllocator or here
// TODO: implement proper config file loading

#define core getCore()

static constexpr char configFileName[] = "config";

bool Storage::startup()
{
	diskWorkerThread.create(DiskWorkerThreadMain, this);
}

void Storage::createSolutionAsync(const char* source, uint32 sourceLength,
	Language language, ProblemId problemId, TestingPolicy testingPolicy)
{
	Problem *problem = problemCache.find(problemId);

	Solution *solution = solutionAllocator.allocate();
	solution->problemId = problemId;
	solution->language = language;
	solution->testingPolicy = testingPolicy;
	solution->state = SolutionState::Waiting;
	solution->source = source;
	solution->sourceLength = sourceLength;
	solution->result = { 0 };

	if (problem)
	{
		solutionCache.insert(solution);
		core.onStorageSolutionCreationComplete(SubmitSolutionResult::Success, solution);
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
		bool result = true;
		result &= loadConfigFile();
		result &= solutionFiles.open();
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
				ProblemId problemId = args.solution->getProblemId();

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
					core.onStorageSolutionCreationComplete(SubmitSolutionResult::Success, args.solution);
				}
				else
				{
					solutionAllocator.release(args.solution);
					core.onStorageSolutionCreationComplete(SubmitSolutionResult::SystemError, args.solution);
				}

				break;
			}

		default:
			Debug::Crash(DbgMsgFmt("invalid DiskWorkerQueueItem::Type"));
		}
	}

	core.onStorageShutdownComplete();
}

inline bool Storage::loadConfigFile()
{
	FILE *file = fopen(configFileName, "r");
	if (!file)
		return false;

	uint32 addressCount = 0;
	fscanf(file, "%d", &addressCount);
	
	if (addressCount > 0xFF)
	{
		fclose(file);
		return false;
	}

	IPv4Address addressBuffer[256];
	for (uint32 i = 0; i < addressCount; i++)
	{
		uint32 a, b, c, d;
		if (fscanf(file, "%d.%d.%d.%d", &a, &b, &c, &d) != 4)
		{
			fclose(file);
			return false;
		}
		addressBuffer[i] = IPv4Address(a, b, c, d);
	}

	core.onStorageConfigFileLoaded(addressBuffer, uint8(addressCount));
	
	fclose(file);
	return true;
}