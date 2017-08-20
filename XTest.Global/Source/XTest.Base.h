#pragma once

#include <XLib.Types.h>

namespace XTest
{
	using XTSolutionId = uint32;
	using XTProblemId = uint32;

	enum class XTLanguage : uint8
	{
		None = 0,
		MSVCPP = 1,
		GNUCPP = 2,
		Pascal = 3,
	};

	enum class XTTestingPolicy : uint8
	{
		None = 0,
		AllTests = 1,
		UntilFirstError = 2,
	};

	enum class XTSolutionState : uint8
	{
		None = 0,
		Waiting,
		Compiling,
		Testing,
		Tested,
	};

	struct XTTestRunInfo
	{
		uint memory : 14;
		uint cycles : 14;
		uint result : 4;
	};

	static constexpr XTSolutionId invalidSolutionId = XTSolutionId(-1);
	static constexpr XTProblemId invalidProblemId = XTProblemId(-1);
	static constexpr uint32 solutionSourceLengthLimit = 0x10000;

	inline bool XTIsValidSolutionId(XTProblemId id) { return id != invalidSolutionId; }
	inline bool XTIsValidProblemId(XTProblemId id) { return id != invalidProblemId; }
}