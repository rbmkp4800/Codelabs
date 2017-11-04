#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.File.h>

#include "XTest.Base.h"
#include "XTest.Manager.Internal.Predefine.h"

namespace XTest::Manager::_Core::_Storage
{
	class SolutionFiles : public XLib::NonCopyable
	{
	private:
		static constexpr uint32 writeQueueSizeLog2 = 4;
		static constexpr uint32 writeQueueSize = 1 << writeQueueSizeLog2;
		static constexpr uint32 writeQueueIndexMask = writeQueueSize - 1;

		Internal::Solution *writeQueue[writeQueueSize];
		volatile XTSolutionId reservedSolutionId, queuedSolutionId, storedSolutionId;

		uint64 workspaceId;
		XLib::File indexFile;
		//XLib::File tempIndexFile;
		//XLib::File tempSourcesFile, compressedSourcesFile;

	public:
		static void CreateEmpty();

		bool open();
		void close();

		void enqueueSolution(Internal::Solution* solution); // fills solution id

		void syncFlush();
		//void syncUpdateIndex(SolutionId solutionId, ProblemId problemId,
		//	TestingPolicy testingPolicy, uint32 index);

		inline uint64 getWorkspaceId() { return workspaceId; }
	};
}