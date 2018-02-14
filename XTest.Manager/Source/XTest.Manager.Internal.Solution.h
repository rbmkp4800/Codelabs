#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>

#include "XTest.Base.h"

namespace XTest::Manager::_Core { class Storage; }

namespace XTest::Manager::Internal
{
	class Solution : public XLib::NonCopyable
	{
		friend class _Core::Storage;

	private:
		XTSolutionId id;
		XTProblemId problemId;
		const Problem *problem;
		XTLanguage language;
		XTTestingPolicy testingPolicy;
		XTSolutionState state;

		const char *source;
		uint32 sourceLength;

		union
		{
			struct
			{
				XTTestRunInfo *testingInfo;
				uint16 ranTestCount;
			};

			struct
			{
				char *error;
				uint16 errorLength;
			};
		} result;

		Solution() = default;

	public:
		inline XTSolutionId getId() const { return id; }
		inline XTProblemId getProblemId() const { return problemId; }
		inline const Problem* getProblem() const { return problem; }
		inline XTLanguage getLanguage() const { return language; }
		inline XTTestingPolicy getTestingPolicy() const { return testingPolicy; }
		inline XTSolutionState getState() const { return state; }

		inline const char* getSource() const { return source; }
		inline uint32 getSourceLength() const { return sourceLength; }
	};
}