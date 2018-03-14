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

void Storage::asyncCreateSolution(const char* source, uint32 sourceLength,
	Language language, ProblemId problemId, TestingPolicy testingPolicy)
{
	Problem *problem = problemCache.find(problemId);

	Solution *solution = solutionAllocator.allocate();
	solution->problemId = problemId;
	solution->problem = problem;
	solution->language = language;
	solution->testingPolicy = testingPolicy;
	solution->state = SolutionState::Waiting;
	solution->source = source;
	solution->sourceLength = sourceLength;
	solution->result = { };

	DiskWorkerQueueItem item;
	item.type = DiskWorkerQueueItem::Type::CreateSolutionRecord;
	item.createSolutionRecord.solution = solution;
	diskWorkerQueue.enqueue(item);
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
			case DiskWorkerQueueItem::Type::CreateSolutionRecord:
				diskWorker_createSolutionRecord(item.createSolutionRecord.solution);
				break;

			default:
				Debug::Crash(DbgMsgFmt("invalid DiskWorkerQueueItem::Type"));
		}
	}

	core.onStorageShutdownComplete();
}

inline void Storage::diskWorker_createSolutionRecord(Solution* const solution)
{
	Debug::CrashConditionOnDebug(solution->getSource() == nullptr,
		DbgMsgFmt("invalid solution (no source attached)"));

	ProblemId problemId = solution->getProblemId();
	Problem *problem = solution->getProblem();

	// check if problem was loaded when solution was submitted
	if (problem)
		goto label_createSolutionRecordAndReportResult;

	// check if problem was loaded while this solution processing was in disk queue
	Problem *problem = problemCache.find(problemId);
	if (problem)
		goto label_createSolutionRecordAndReportResult;

	// find free or least recently used file
	ProblemFile *freeFile = nullptr;
	for each (ProblemFile& file in problemFiles)
	{
		if (!file.isOpened())
		{
			freeFile = &file;
			break;
		}
	}

	if (!freeFile)
	{
		// TODO: add handling for situations when least recently used file has some
		//		pending IO operations

		freeFile = ...; // find least recently used file
		freeFile->close();
	}

	// try to open problem
	problem = problemAllocator.allocate();
	if (freeFile->open(problemId, *problem))
		goto label_createSolutionRecordAndReportResult;

	// problem file does not exist or is corrupted. Can't create solution
	problemAllocator.release(problem);
	solutionAllocator.release(solution);
	core.onStorageSolutionCreationComplete(SubmitSolutionResult::SystemError, solution);
	return;


label_createSolutionRecordAndReportResult:
	solutionFiles.syncCreateSolutionRecord(problemId, solution->getLanguage(),
		solution->getTestingPolicy(), solution->getSource(), solution->getSourceLength());

	core.onStorageSolutionCreationComplete(SubmitSolutionResult::Success, solution);
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