#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.Containers.Set.h>
#include <XLib.System.Threading.h>
#include <XLib.System.Threading.CyclicQueue.h>
//#include <XLib.System.Threading.ReadersWriterLock.h>

#include "XTest.Base.h"
#include "XTest.Manager.Internal.Solution.h"
#include "XTest.Manager.Internal.Problem.h"
#include "XTest.Manager.Core.Storage.SolutionFiles.h"
#include "XTest.Manager.Core.Storage.ProblemFile.h"

namespace XTest::Manager { class XTMCore; }

namespace XTest::Manager::_Core
{
	class Storage : public XLib::NonCopyable
	{
	private:
		using SolutionCache = XLib::Set<Internal::Solution, XLib::SetStoragePolicy::InternalHeapBuffer<5, 10>>;
		using ProblemCache = XLib::Set<Internal::Problem, XLib::SetStoragePolicy::InternalHeapBuffer<5, 8>>;

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

		_Storage::SolutionFiles solutionFiles;
		_Storage::ProblemFile problemFiles[openedProblemFilesLimit];

		// TODO: check if we need this locks
		//XLib::ReadersWriterLock solutionCacheLock;
		//XLib::ReadersWriterLock problemCacheLock;
		SolutionCache solutionCache;
		ProblemCache problemCache;

		DiskWorkerQueue diskWorkerQueue;
		XLib::Thread diskWorkerThread;
		static uint32 __stdcall DiskWorkerThreadMain(Storage* self);
		void diskWorkerThreadMain();

		inline XTMCore& getCore();

	public:
		bool startup();

		void createSolution(const char* source, uint32 sourceLength,
			XTLanguage language, XTProblemId problemId, XTTestingPolicy testingPolicy);
	};
}