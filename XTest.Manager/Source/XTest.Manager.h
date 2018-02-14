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
// TODO: implement proper storage startup result handling (and results enum)

namespace XTest::Manager::_Core { class Worker; }
namespace XTest::Manager::Internal { class Solution; }

namespace XTest::Manager
{
	enum class SubmitSolutionResult : uint8
	{
		Success = 0,

		InvalidState,
		InvalidSourceLength,
		InvalidLanguage,
		InvalidProblemId,
		InvalidTestingPolicy,
		ProblemNotFound,
		SystemError,
	};

	class ManagerCallbacks
	{
		friend class ManagerCore;

	protected:
		void onStartupComplete();
		void onShutdownComplete();
		void onSolutionSubmitComplete(SubmitSolutionResult result, SolutionId id);
		void onSolutionStateUpdated();
		// void onSolutionTestingResultsLoaded();
		// ...
	};

	class ManagerCore : public XLib::NonCopyable
	{
		friend _Core::Worker;
		friend _Core::Storage;

	private: // meta
		enum class State : uint8
		{
			Down = 0,
			Startup,
			Active,
			Shutdown,
		};

		struct WorkerDesc
		{
			XLib::IPv4Address address;
			_Core::Worker *core;
		};

		using SolutionTestingQueue = XLib::CyclicQueue<Internal::Solution*,
			XLib::CyclicQueueStoragePolicy::InternalStatic<256>>;

	private: // data
		_Core::Storage storage;

		XLib::AsyncIODispatcher dispatcher;
		XLib::Thread dispatcherThread;

		XLib::TCPListenSocket workersListenSocket;
		XLib::TCPListenSocket::DispatchedAsyncTask workersListenTask;

		SolutionTestingQueue solutionTestingQueue;
		XLib::HeapPtr<WorkerDesc> workers;

		ManagerCallbacks *callbacks = nullptr;

		State state = State::Down;
		uint8 workerCount = 0;

	private: // code
		void onWorkerSocketAccepted(bool result,
			XLib::TCPSocket& socket,
			XLib::IPAddress address, uintptr);
		void onWorkerDisconnected(uint8 workerId);
		void onWorkerSlotReady(uint8 workerId);
		void onWorkerSolutionStateUpdated(Internal::Solution* solution);

		void onStorageConfigFileLoaded(
			const XLib::IPv4Address* workerAddresses, uint8 workerAddressCount);
		void onStorageStartupComplete(bool result);
		void onStorageShutdownComplete();
		void onStorageSolutionCreationComplete(
			SubmitSolutionResult result, Internal::Solution* solution);

		static uint32 __stdcall DispatcherThreadMain(ManagerCore* self);
		void dispatcherThreadMain();

	public:
		ManagerCore() = default;
		~ManagerCore();

		bool startup(ManagerCallbacks* callbacks,
			const char* workspacePath, uint16 workersListenPort);
		void shutdown();

		void submitSolution(const char* source, uint32 sourceLength,
			Language language, ProblemId problemId, TestingPolicy testingPolicy);

		uint64 getWorkspaceId();
		//void loadSolutionTestingResult();

		inline bool isRunning() { /* ... */ }
	};
}