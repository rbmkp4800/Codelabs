#pragma once

#include <XLib.Types.h>

namespace XTest
{
	using SolutionId = uint32;
	using ProblemId = uint32;

	enum class Language : uint8
	{
		None = 0,
		MSVCPP = 1,
		GNUCPP = 2,
		Pascal = 3,
	};

	enum class TestingPolicy : uint8
	{
		None = 0,
		AllTests = 1,
		UntilFirstError = 2,
	};

	enum class SolutionState : uint8
	{
		None = 0,
		Waiting,
		Compiling,
		Testing,
		Tested,
	};

	struct TestRunInfo
	{
		uint memory : 14;
		uint cycles : 14;
		uint result : 4;
	};

	static constexpr SolutionId invalidSolutionId = SolutionId(-1);
	static constexpr ProblemId invalidProblemId = ProblemId(-1);
	static constexpr uint32 solutionSourceLengthLimit = 0x10000;

	inline bool IsValidSolutionId(ProblemId id) { return id != invalidSolutionId; }
	inline bool IsValidProblemId(ProblemId id) { return id != invalidProblemId; }
}