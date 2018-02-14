#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.PoolAllocator.h>
#include <XLib.Containers.IntrusiveSet.h>
#include <XLib.System.Threading.h>
#include <XLib.System.Threading.CyclicQueue.h>
//#include <XLib.System.Threading.ReadersWriterLock.h>

#include "XTest.Base.h"
#include "XTest.Manager.Internal.Solution.h"
#include "XTest.Manager.Internal.Problem.h"
#include "XTest.Manager.Core.Storage.SolutionFiles.h"
#include "XTest.Manager.Core.Storage.ProblemFile.h"

// TODO: check if we need RW lock on solution and problem caches

namespace XTest::Manager { class XTMCore; }

namespace XTest::Manager::_Core
{
	class Storage : public XLib::NonCopyable
	{
	private: // meta
		using SolutionAllocator = XLib::PoolAllocator<Internal::Solution,
			XLib::PoolAllocatorHeapUsagePolicy::MultipleStaticChunks<5, 12>>;
		using ProblemAllocator = XLib::PoolAllocator<Internal::Problem,
			XLib::PoolAllocatorHeapUsagePolicy::MultipleStaticChunks<5, 12>>;

		//using SolutionCache = XLib::IntrusiveSet<Internal::Solution, >;
		//using ProblemCache = XLib::IntrusiveSet<Internal::Problem, >;

		enum class DiskWorkerQueueItemType : uint8
		{
			None = 0,
			CheckProblemFileAndCompleteSolutionCreation = 1,
		};

		struct DiskWorkerQueueItem
		{
			DiskWorkerQueueItemType type;

			union
			{
				Solution* preAllocatedSolution;
			} checkProblemFileAndCompleteSolutionCreation;
		};

		using DiskWorkerQueue = XLib::ThreadSafeCyclicQueue<DiskWorkerQueueItem, 6,
			XLib::ThreadSafeQueueType::MultipleProducersSingleConsumer>;

		static constexpr uint32 openedProblemFilesLimit = 16;

	private: // data
		SolutionAllocator solutionAllocator;
		ProblemAllocator problemAllocator;
		//SolutionCache solutionCache;
		//ProblemCache problemCache;

		_Storage::SolutionFiles solutionFiles;
		_Storage::ProblemFile problemFiles[openedProblemFilesLimit];

		DiskWorkerQueue diskWorkerQueue;
		XLib::Thread diskWorkerThread;

	private: // code
		static uint32 __stdcall DiskWorkerThreadMain(Storage* self);
		void diskWorkerThreadMain();

		inline XTMCore& getCore();

	public:
		bool startup();
		void shutdown();

		void createSolution(const char* source, uint32 sourceLength,
			XTLanguage language, XTProblemId problemId, XTTestingPolicy testingPolicy);

		void fetchSolution();
	};
}