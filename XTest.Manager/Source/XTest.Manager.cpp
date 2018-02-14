#include <XLib.Heap.h>
#include <XLib.Debug.h>

#include "XTest.Manager.h"
#include "XTest.Manager.Core.Worker.h"

// TODO: refactor workers objects allocation

// Scheduler logic:
//		

using namespace XLib;
using namespace XTest::Manager;
using namespace XTest::Manager::Internal;
using namespace XTest::Manager::_Core;

XTMCore::~XTMCore()
{
	Debug::CrashCondition(isRunning(), DbgMsgFmt("object must be shut down properly"));
}

// workers ==================================================================================//

void XTMCore::onWorkerSocketAccepted(bool result, TCPSocket& socket, IPAddress address, uintptr)
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

void XTMCore::onWorkerDisconnected(uint8 workerId)
{

}

void XTMCore::onWorkerSlotReady(uint8 workerId)
{
	// call scheduler. Try to pull solution from queue
}

void XTMCore::onWorkerSolutionStateUpdated(Internal::Solution* solution)
{

}

// storage ==================================================================================//

void XTMCore::onStorageStartupComplete(bool result)
{
	callbacks->onStartupComplete(/*result*/);
}

void XTMCore::onStorageShutdownComplete()
{
	for (uint8 i = 0; i < workerCount; i++)
	{
		if (workers[i].core)
			Heap::Release(workers[i].core);
	}
	callbacks->onShutdownComplete();
}

void XTMCore::onStorageSolutionCreationComplete(
	XTMSubmitSolutionResult result, Internal::Solution* solution)
{
	callbacks->onSolutionSubmitComplete(result, solution->getId());

	// call scheduler. Try to push solution to one of the workers or put it to the queue
}

// public interface =========================================================================//

bool XTMCore::startup(XTMCoreCallbacks* callbacks,
	const char* workspacePath, uint16 workersListenPort)
{
	workersListenSocket.initialize(IPv4AddressAny, workersListenPort);
	dispatcher.associate(workersListenSocket);
	workersListenSocket.start();
	workersListenSocket.asyncAccept(workersListenTask,
		SocketAcceptedHandler(*this, &XTMCore::onWorkerSocketAccepted));

	dispatcherThread.create(DispatcherThreadMain, this);

	return true;
}

void XTMCore::shutdown()
{
	// proper shutdown
}

void XTMCore::submitSolution(const char* source, uint32 sourceLength, XTLanguage language,
	XTProblemId problemId, XTTestingPolicy testingPolicy, void* context)
{
	if (sourceLength > solutionSourceLengthLimit)
		callbacks->onSolutionSubmitComplete(XTMSubmitSolutionResult::InvalidSourceLength, invalidSolutionId);
	if (!XTIsValidProblemId(problemId))
		callbacks->onSolutionSubmitComplete(XTMSubmitSolutionResult::InvalidProblemId, invalidSolutionId);
	// TODO: check testingPolicy

	storage.createSolutionAsync(source, sourceLength, language, problemId, testingPolicy);
}