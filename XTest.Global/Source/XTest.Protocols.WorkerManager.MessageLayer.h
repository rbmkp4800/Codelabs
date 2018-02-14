#pragma once

#include <XLib.Types.h>

#include "XTest.Base.h"

// WorkerInitRequest          M -> W
//		protocol version
//----------------------------------
// WorkerInitResponse         M <- W
//		worker version
//		slot count
//		machine config
//----------------------------------
// TestSolutionRequest        M -> W
//		slot id
//		problem id
//		language
//		testing policy
//		checker version
//		source (extension)
//----------------------------------
// TestSolutionResponse       M <- W
//		slot id
//		result
//----------------------------------
// SolutionCompiled           M <- W
//		slot id
//----------------------------------
// SolutionCompilationError   M <- W
//		slot id
//		compiler output (extension)
//----------------------------------
// SolutionTestRunInfo        M <- W
//		slot id
//		test run info
//		is final
//----------------------------------
// TestingSystemError         M <- W
//		slot id
//		error code
//----------------------------------
// GetCheckerRequest          M <- W
//		slot id ???
//----------------------------------
// GetCheckerResponse         M -> W
//		slot id ???
//		checker source (extension)
//----------------------------------
// CheckerCompilationError    M <- W
//		problem id
//		compiler output (extension)

#pragma pack(push, 1)

namespace XTest::Protocols::WorkerManager::MessageLayer
{
	class ProtocolConstants abstract final
	{
	public:
		static constexpr uint16 Version = 0x0001;
	};

	enum class PacketType : uint8
	{
		None = 0,

		WorkerInitRequest,
		WorkerInitResponse,

		TestSolutionRequest,
		TestSolutionResponse,
		SolutionCompiled,
		SolutionCompilationError,
		SolutionTestRunInfo,
		TestingSystemError,

		GetCheckerRequest,
		GetCheckerResponse,
		CheckerCompilationError,
	};

	struct Packet abstract final
	{
		struct WorkerInitRequest
		{
			PacketType type;

			uint16 protocolVersion;

			inline WorkerInitRequest() : type(PacketType::WorkerInitRequest) {}
		};

		struct WorkerInitResponse
		{
			PacketType type;

			uint16 workerVersion;
			uint8 slotCount;
			// char machineConfig[];

			inline WorkerInitResponse() : type(PacketType::WorkerInitResponse) {}
		};

		struct TestSolutionRequest
		{
			PacketType type;

			uint8 slotId;
			XTProblemId problemId;
			XTLanguage language;
			XTTestingPolicy testingPolicy;
			uint32 checkerVersion;

			inline TestSolutionRequest() : type(PacketType::TestSolutionRequest) {}
		};
	};
}

#pragma pack(pop)