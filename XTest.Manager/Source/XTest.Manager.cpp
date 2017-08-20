//#include <XLib.Heap.h>
#include <XLib.Debug.h>

#include "XTest.Manager.h"

using namespace XLib;
using namespace XTest::Manager;
using namespace XTest::Manager::Internal;
using namespace XTest::Manager::_Core;

void XTMCore::onWorkerSocketAccepted(bool result, TCPSocket& socket, IPAddress address, uintptr)
{
	if (!result)
	{
		Debug::Warning(DbgMsgFmt("listen socket failure"));
		return;
	}

	for (uint32 i = 0; i < workersLimit; i++)
	{
		if (workerDescs[i].address == address)
		{
			if (workers[i])
			{
				// worker already exists
			}
			else
			{
				//workers[i] = Heap::Allocate<
			}

			break;
		}
	}
}

// storage ==================================================================================//

void XTMCore::onStorageStartupComplete(bool result)
{
	callbacks->onStartupComplete(/*result*/);
}

void XTMCore::onStorageShutdownComplete()
{
	callbacks->onShutdownComplete();
}

void XTMCore::onStorageSolutionCreationComplete(
	XTMSubmitSolutionResult result, Internal::Solution* solution)
{
	callbacks->onSolutionSubmitComplete();
}

// workers ==================================================================================//

void XTMCore::onWorkerInitComplete(uint8 workerId)
{

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

void XTMCore::submitSolution(const char* source, uint32 sourceLength, XTLanguage language,
	XTProblemId problemId, XTTestingPolicy testingPolicy, void* context)
{
	if (sourceLength > solutionSourceLengthLimit)
		callbacks->onSolutionSubmitComplete(XTMSubmitSolutionResult::InvalidSourceLength, invalidSolutionId);
	if (!XTIsValidProblemId(problemId))
		callbacks->onSolutionSubmitComplete(XTMSubmitSolutionResult::InvalidProblemId, invalidSolutionId);
	// TODO: check testingPolicy

	// when complete calls onSolutionCreationComplete
	storage.createSolution(source, sourceLength, language, problemId, testingPolicy);
}