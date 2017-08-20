#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>
#include <XLib.System.File.h>

#include "XTest.Base.h"

namespace XTest::Manager::_Core::_Storage
{
	class ProblemFile : public XLib::NonCopyable
	{
	private:
		XLib::File file;
		XTProblemId problemId = invalidProblemId;
		uint32 recordCount = 0;
		uint16 testCount = 0;

	public:
		bool open(XTProblemId problemId);
		void close();

		inline bool isOpened() { return file.isInitialized(); }
		inline XTProblemId getProblemId() { return problemId; }
	};
}