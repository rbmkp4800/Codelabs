#include <XLib.Debug.h>
#include <XLib.StringWriter.h>
#include <XLib.Crypto.CRC.h>

#include "XTest.Manager.Core.Storage.ProblemFile.h"

#include "XTest.Manager.Internal.Problem.h"

// TODO: deal with problem.points allocation
// TODO: add disk operations error handling

using namespace XLib;
using namespace XTest;
using namespace XTest::Manager::_Core::_Storage;
using namespace XTest::Manager::Internal;

#pragma pack(push, 4)
namespace
{
	class FileConfig abstract final
	{
	public:
		static constexpr uint64 MagicValue;
		static constexpr uint16 SupportedVersion = 0x0001;
	};

	struct FileHeader
	{
		uint64 magic;
		uint16 version;
		uint64 workspaceId;
		uint16 testCount;
		uint16 checkerVersion;
	};

	// struct FileRecord (aligned to 4 bytes)
	//		SolutionId solutionId;
	//		TestRunInfo testingInfo[testCount];
	//		uint32 recordChecksum;

	static_assert(sizeof(TestRunInfo) % 4 == 0, "TestRunInfo must be aligned to 4 bytes");
}
#pragma pack(pop)

bool ProblemFile::open(ProblemId problemId, Problem& problem)
{
	Debug::CrashConditionOnDebug(file.isInitialized(), DbgMsgFmt("already opened"));

	const char* localFailMessage = nullptr;

	{
		char filename[64];
		StringWriter writer(filename);
		writer.put("problem.", FmtHex<0, true>(problemId), endOfString);

		if (!file.open(filename, FileAccessMode::ReadWrite, FileOpenMode::OpenExisting))
		{
			// if file not found - invalid ProblemId
			// else - internal error
			Debug::Warning(DbgMsgFmt("error opening problem file"));
			return false;
		}
	}

	FileHeader header;
	if (!file.read(header))
	{
		Debug::Warning(DbgMsgFmt("error reading index file"));
		return false;
	}
	if (header.magic != FileConfig::MagicValue)
	{
		Debug::Warning(DbgMsgFmt("invalid file magic"));
		return false;
	}
	if (header.version != FileConfig::SupportedVersion)
	{
		Debug::Warning(DbgMsgFmt("unsupported file version"));
		return false;
	}
	if (header.testCount == 0)
	{
		Debug::Warning(DbgMsgFmt("invalid test count"));
		return false;
	}

	// TODO: add workspaceId check

	uint64 recordsArraySize = file.getSize() - sizeof(FileHeader);
	uint32 recordSize = sizeof(SolutionId) + sizeof(TestRunInfo) * header.testCount + sizeof(uint32);
	if (recordsArraySize % recordSize != 0)
	{
		Debug::Warning(DbgMsgFmt("file corrupted"));
		return false;
	}

	problem.id = problemId;
	problem.checkerVersion = header.checkerVersion;
	problem.testCount = header.testCount;
	//problem.points = ;

	this->problem = &problem;
	this->recordCount = recordsArraySize / recordSize;;
	this->testCount = header.testCount;
	this->endOfFile = true;

	return true;
}

void ProblemFile::close()
{
	if (problem != nullptr)
	{
		file.close();

		problem = nullptr;
		recordCount = 0;
		testCount = 0;
		endOfFile = false;
	}
}

uint32 ProblemFile::syncWriteTestingInfo(SolutionId solutionId, const TestRunInfo* testingInfo)
{
	Debug::CrashConditionOnDebug(problem == nullptr, DbgMsgFmt("not initialized"));

	CRC32 crc;
	crc.process(solutionId);
	crc.process(testingInfo, testCount * sizeof(TestRunInfo));

	if (!endOfFile)
	{
		file.setPosition(0, FilePosition::End);
		endOfFile = true;
	}

	file.write(solutionId);
	file.write(testingInfo, testCount * sizeof(TestRunInfo));
	file.write(crc.getValue());

	uint32 recordId = recordCount;
	recordCount++;

	return recordId;
}

bool ProblemFile::syncReadTestingInfo(uint32 recordIndex, TestRunInfo* testingInfo,
	SolutionId& solutionId)
{
	Debug::CrashConditionOnDebug(problem == nullptr, DbgMsgFmt("not initialized"));
	
	if (recordIndex >= recordCount)
		return false;

	const uint32 recordSize = sizeof(SolutionId) + sizeof(TestRunInfo) * testCount + sizeof(uint32);
	file.setPosition(sizeof(FileHeader) + recordSize * recordIndex);
	endOfFile = false;

	uint32 crcCheck = 0;
	file.read(solutionId);
	file.read(testingInfo, testCount * sizeof(TestRunInfo));
	file.read(crcCheck);
	
	CRC32 crc;
	crc.process(solutionId);
	crc.process(testingInfo, testCount * sizeof(TestRunInfo));

	return crc.getValue() == crcCheck;
}