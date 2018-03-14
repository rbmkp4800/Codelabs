#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.File.h>

#include "XTest.Base.h"

namespace XTest::Manager::_Core::_Storage
{
	class SolutionFiles : public XLib::NonCopyable
	{
	private:
		XLib::File indexFile;
		XLib::File sourcesFile;
		uint32 totalSolutionCount = 0;

	public:
		bool open();
		void close();

		SolutionId syncCreateSolutionRecord(ProblemId problemId, Language language,
			TestingPolicy testingPolicy, const char* source, uint32 sourceLength);
	};
}