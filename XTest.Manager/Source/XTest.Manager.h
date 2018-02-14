#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.Delegate.h>
#include <XLib.Containers.CyclicQueue.h>
#include <XLib.System.Threading.h>
#include <XLib.System.AsyncIO.h>
#include <XLib.System.Network.Socket.h>

#include "XTest.Base.h"
#include "XTest.Manager.Core.Storage.h"

// TODO: implement SolutionsTestingQueue using smth like ExtendableCyclicQueue

namespace XTest::Manager::_Core { class Worker; }
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

	private: // meta
		struct WorkerDesc
		{
			XLib::IPv4Address address;
			_Core::Worker *core;
		};

		using SolutionsTestingQueue = XLib::CyclicQueue<Internal::Solution*,
			XLib::CyclicQueueStoragePolicy::InternalStatic<256>>;

	private: // data
		_Core::Storage storage;

		XLib::AsyncIODispatcher dispatcher;
		XLib::Thread dispatcherThread;

		XLib::TCPListenSocket workersListenSocket;
		XLib::TCPListenSocket::DispatchedAsyncTask workersListenTask;

		SolutionsTestingQueue solutionsTestingQueue;
		XLib::HeapPtr<WorkerDesc> workers;

		XTMCoreCallbacks *callbacks = nullptr;

		uint8 workerCount = 0;

	private: // code
		void onWorkerSocketAccepted(bool result,
			XLib::TCPSocket& socket,
			XLib::IPAddress address, uintptr);
		void onWorkerDisconnected(uint8 workerId);
		void onWorkerSlotReady(uint8 workerId);
		void onWorkerSolutionStateUpdated(Internal::Solution* solution);

		void onStorageStartupComplete(bool result);
		void onStorageShutdownComplete();
		void onStorageSolutionCreationComplete(
			XTMSubmitSolutionResult result, Internal::Solution* solution);

		static uint32 __stdcall DispatcherThreadMain(XTMCore* self);
		void dispatcherThreadMain();

	public:
		XTMCore() = default;
		~XTMCore();

		bool startup(XTMCoreCallbacks* callbacks, const char* workspacePath, uint16 workersListenPort);
		void shutdown();

		void submitSolution(const char* source, uint32 sourceLength, XTLanguage language,
			XTProblemId problemId, XTTestingPolicy testingPolicy, void* context);

		uint64 getWorkspaceId();
		//void loadSolutionTestingResult();

		inline bool isRunning() { /* ... */ }
	};
}