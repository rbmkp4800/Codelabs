#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>

#include "XTest.Base.h"

namespace XTest::Manager::_Core { class Storage; }
namespace XTest::Manager::Internal { class Problem; }

namespace XTest::Manager::Internal
{
	class Solution : public XLib::NonCopyable
	{
		friend class _Core::Storage;

	private:
		SolutionId id;
		ProblemId problemId;
		const Problem *problem;
		Language language;
		TestingPolicy testingPolicy;
		SolutionState state;

		const char *source;
		uint32 sourceLength;

		union
		{
			TestRunInfo *testingInfo;

			struct
			{
				char *error;
				uint16 errorLength;
			};
		} result;

		Solution() = default;

	public:
		inline SolutionId getId() const { return id; }
		inline ProblemId getProblemId() const { return problemId; }
		inline const Problem* getProblem() const { return problem; }
		inline Language getLanguage() const { return language; }
		inline TestingPolicy getTestingPolicy() const { return testingPolicy; }
		inline SolutionState getState() const { return state; }

		inline const char* getSource() const { return source; }
		inline uint32 getSourceLength() const { return sourceLength; }

		inline const TestRunInfo* getTestingInfo() const { return result.testingInfo; }
	};
}