#include <XLib.Debug.h>
#include <XLib.StringWriter.h>

#include "XTest.Manager.Core.Storage.ProblemFile.h"

using namespace XLib;
using namespace XTest;
using namespace XTest::Manager::_Core::_Storage;

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

bool ProblemFile::open(ProblemId problemId)
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

	this->problemId = problemId;
	this->recordCount = recordsArraySize / recordSize;;
	this->testCount = header.testCount;

	return true;
}

void ProblemFile::close()
{
	if (file.isInitialized())
	{
		file.close();

		problemId = invalidProblemId;
		recordCount = 0;
		testCount = 0;
	}
}