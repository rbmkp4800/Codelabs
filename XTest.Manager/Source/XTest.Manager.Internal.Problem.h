#pragma once

#include <XLib.Types.h>
#include <XLib.NonCopyable.h>

#include "XTest.Base.h"

namespace XTest::Manager::_Core { class Storage; }

namespace XTest::Manager::Internal
{
	class Problem : public XLib::NonCopyable
	{
		friend class _Core::Storage;

	private:
		ProblemId id = invalidProblemId;
		uint16 checkerVersion = 0;
		uint16 testCount = 0;
		uint8 *points = nullptr;

		Problem() = default;

	public:
		inline ProblemId getId() const { return id; }
	};

	inline bool operator > (ProblemId leftProblemId, const Problem& rightProblem) { return leftProblemId > rightProblem.getId(); }
	inline bool operator > (const Problem& leftProblem, ProblemId rightProblemId) { return leftProblem.getId() > rightProblemId; }
}