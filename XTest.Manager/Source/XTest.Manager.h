#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.Delegate.h>
#include <XLib.Containers.CyclicQueue.h>
#include <XLib.System.Threading.h>
#include <XLib.System.AsyncIO.h>
#include <XLib.System.Network.Socket.h>

#include "XTest.Base.h"
#include "XTest.Manager.Core.Worker.h"
#include "XTest.Manager.Core.Storage.h"

namespace XTest::Manager::Internal { class Solution; }

namespace XTest::Manager
{
	enum class XTMSubmitSolutionResult : uint8
	{
		Success = 0,

		InvalidSourceLength,
		InvalidLanguage,
		InvalidProblemId,
		InvalidTestingPolicy,
		ProblemNotFound,
		SystemError,
	};

	class XTMCoreCallbacks
	{
		friend class XTMCore;

	protected:
		void onStartupComplete();
		void onShutdownComplete();
		void onSolutionSubmitComplete(XTMSubmitSolutionResult result, XTSolutionId id);
		void onSolutionStateUpdated();
		// void onSolutionTestingResultsLoaded();
		// ...
	};

	class XTMCore : public XLib::NonCopyable
	{
		friend _Core::Worker;
		friend _Core::Storage;

	private:
		struct WorkerDesc
		{
			XLib::IPv4Address address;
		};

		using SolutionsTestingQueue = XLib::CyclicQueue<Internal::Solution*,
			XLib::CyclicQueueStoragePolicy::InternalStatic<256>>;

		static constexpr uint32 workersLimit = 4;

		//-----------------------------------------------------------------------

		XLib::AsyncIODispatcher dispatcher;
		XLib::Thread dispatcherThread;
		void dispatcherThreadMain();

		XLib::TCPListenSocket workersListenSocket;
		XLib::TCPListenSocket::DispatchedAsyncTask workersListenTask;
		void onWorkerSocketAccepted(bool result, XLib::TCPSocket& socket,
			XLib::IPAddress address, uintptr);

		WorkerDesc workerDescs[workersLimit];
		_Core::Worker *workers[workersLimit];
		void onWorkerInitComplete(uint8 workerId);
		void onWorkerDisconnected(uint8 workerId); // releases worker
		void onWorkerSolutionStateUpdated(Internal::Solution* solution);

		_Core::Storage storage;
		void onStorageStartupComplete(bool result);
		void onStorageShutdownComplete();
		void onStorageSolutionCreationComplete(XTMSubmitSolutionResult result, Internal::Solution* solution);

		XLib::AsyncIODispatcher dispatcher;
		XLib::Thread dispatcherThread;
		static uint32 __stdcall DispatcherThreadMain(XTMCore* self);
		void dispatcherThreadMain();

		XTMCoreCallbacks *callbacks = nullptr;

	public:
		bool startup(XTMCoreCallbacks* callbacks, const char* workspacePath, uint16 workersListenPort);
		void shutdown();

		void submitSolution(const char* source, uint32 sourceLength, XTLanguage language,
			XTProblemId problemId, XTTestingPolicy testingPolicy, void* context);
		uint64 getWorkspaceId();
		//void loadSolutionTestingResult();
	};
}