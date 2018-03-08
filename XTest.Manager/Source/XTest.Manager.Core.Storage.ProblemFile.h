#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.File.h>

#include "XTest.Base.h"

// TODO: syncReadTestingInfo result must be different for corrupted file and for disk IO failure

namespace XTest::Manager::Internal { class Problem; }

namespace XTest::Manager::_Core::_Storage
{
	class ProblemFile : public XLib::NonCopyable
	{
	private:
		XLib::File file;
		Internal::Problem *problem = nullptr;
		uint32 recordCount = 0;
		uint16 testCount = 0;
		bool endOfFile = false;

	public:
		bool open(ProblemId problemId, Internal::Problem& problem);
		void close();

		uint32 syncWriteTestingInfo(SolutionId solutionId,
			const TestRunInfo* testingInfo); // -> recordId
		bool syncReadTestingInfo(uint32 recordIndex, TestRunInfo* testingInfo,
			SolutionId& solutionId); // -> fileNotCorrupted

		inline bool isOpened() { return problem != nullptr; }
		inline const Internal::Problem* getProblem() { return problem; }
	};
}