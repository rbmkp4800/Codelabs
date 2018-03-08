#include <XLib.Heap.h>
#include <XLib.Debug.h>

#include "XTest.Manager.h"
#include "XTest.Manager.Core.Worker.h"

// TODO: refactor workers objects allocation
// TODO: handle State::Startup in shutdown()

using namespace XLib;
using namespace XTest::Manager;
using namespace XTest::Manager::Internal;
using namespace XTest::Manager::_Core;

ManagerCore::~ManagerCore()
{
	Debug::CrashCondition(isRunning(), DbgMsgFmt("object must be shut down properly"));
}

// workers ==================================================================================//

void ManagerCore::onWorkerSocketAccepted(bool result,
	TCPSocket& socket, IPAddress address, uintptr)
{
	if (!result)
	{
		Debug::Warning(DbgMsgFmt("listen socket failure"));
		return;
	}

	for (uint8 i = 0; i < workerCount; i++)
	{
		if (workers[i].address != address)
			continue;

		if (workers[i].core)
		{
			Debug::Warning(DbgMsgFmt("trying to accept worker that is already connected"));
			return;
		}

		Worker *worker = Heap::Allocate<Worker>();
		construct(worker);
		worker->initialize(this, i);

		workers[i].core = worker;

		break;
	}
}

void ManagerCore::onWorkerDisconnected(uint8 workerId)
{

}

void ManagerCore::onWorkerSlotReady(uint8 workerId)
{
	if (solutionTestingQueue.isEmpty())
		return;

	Solution *solution = solutionTestingQueue.dequeue();
	workers[workerId].core->pushSolution(solution);
}

void ManagerCore::onWorkerSolutionStateUpdated(Internal::Solution* solution)
{

}

// storage ==================================================================================//

void ManagerCore::onStorageConfigFileLoaded(
	const IPv4Address* workerAddresses, uint8 workerAddressCount)
{
	Debug::CrashConditionOnDebug(state != State::Startup, DbgMsgFmt("invalid state"));

	workers.resize(workerAddressCount);
	workerCount = workerAddressCount;
}

void ManagerCore::onStorageStartupComplete(bool result)
{
	state = State::Active;
	callbacks->onStartupComplete(/*result*/);
}

void ManagerCore::onStorageShutdownComplete()
{
	for (uint8 i = 0; i < workerCount; i++)
	{
		if (workers[i].core)
			Heap::Release(workers[i].core);
	}
	state = State::Down;
	callbacks->onShutdownComplete();
}

void ManagerCore::onStorageSolutionCreationComplete(
	SubmitSolutionResult result, Internal::Solution* solution)
{
	callbacks->onSolutionSubmitComplete(result, solution->getId());

	// select worker with minimum load
	uint8 minLoad = 0xFF;
	Worker *minLoadWorker = nullptr;
	for (uint8 i = 0; i < workerCount; i++)
	{
		Worker &worker = *workers[i].core;

		if (!worker.canAcceptSolution())
			continue;

		uint8 load = uint8(uint16(worker.getFreeSlotCount()) * 0xFF / uint16(worker.getSlotCount()));
		if (minLoad > load)
		{
			minLoad = load;
			minLoadWorker = &worker;
		}
	}

	if (minLoad == 0xFF) // if all slots are busy
		solutionTestingQueue.enqueue(solution);
	else
		minLoadWorker->pushSolution(solution);
}

// public interface =========================================================================//

bool ManagerCore::startup(ManagerCallbacks* callbacks,
	const char* workspacePath, uint16 workersListenPort)
{
	if (state != State::Down)
	{
		Debug::Warning(DbgMsgFmt("invalid state"));
		return false;
	}

	workersListenSocket.initialize(IPv4AddressAny, workersListenPort);
	dispatcher.associate(workersListenSocket);
	workersListenSocket.start();
	workersListenSocket.asyncAccept(workersListenTask,
		SocketAcceptedHandler(this, &ManagerCore::onWorkerSocketAccepted));

	dispatcherThread.create(DispatcherThreadMain, this);

	return storage.startup();
}

void ManagerCore::shutdown()
{
	if (state != State::Active)
	{
		Debug::Warning(DbgMsgFmt("invalid state"));
		return;
	}

	// proper shutdown
}

void ManagerCore::submitSolution(const char* source, uint32 sourceLength,
	Language language, ProblemId problemId, TestingPolicy testingPolicy)
{
	if (state != State::Active)						{ callbacks->onSolutionSubmitComplete(SubmitSolutionResult::InvalidState,			invalidSolutionId); return; }
	if (sourceLength > solutionSourceLengthLimit)	{ callbacks->onSolutionSubmitComplete(SubmitSolutionResult::InvalidSourceLength,	invalidSolutionId); return; }
	if (!IsValidProblemId(problemId))				{ callbacks->onSolutionSubmitComplete(SubmitSolutionResult::InvalidProblemId,		invalidSolutionId); return; }
	// TODO: check testingPolicy

	storage.createSolutionAsync(source, sourceLength, language, problemId, testingPolicy);
}