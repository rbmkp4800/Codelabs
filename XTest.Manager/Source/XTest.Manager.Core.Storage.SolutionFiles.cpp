#include <XLib.Debug.h>
#include "Util.uint24.h"

#include "XTest.Manager.Core.Storage.SolutionFiles.h"

using namespace XLib;
using namespace XTest;
using namespace XTest::Manager::_Core::_Storage;

#pragma pack(push, 1)
namespace
{
	// index file ===========================================================================//

	struct IndexFileRecord;

	class IndexFileConfig abstract final
	{
	public:
		static constexpr uint64 MagicValue;
		static constexpr uint16 SupportedVersion = 0x0001;

		static constexpr uint32 BlockSize = 4096;
		static constexpr uint32 RecordsPerBlock = (BlockSize - sizeof(uint32)) / sizeof(IndexFileRecord);
	};

	struct IndexFileHeader
	{
		uint24 magic;
		uint8 version;
		uint64 workspaceId;
	};

	struct IndexFileRecord
	{
		XTProblemId problemId;
		uint32 index;
	};

	// index temp file ======================================================================//

	struct IndexTempFileBlockHeader
	{
		uint24 magic;
		uint8 version;
		uint32 globalBlockIndex;
	};

	struct IndexTempFileRecord
	{
		IndexFileRecord record;
		uint32 checksum;
	};

	// solutions file =======================================================================//

	class SolutionsFileConfig abstract final
	{
	public:
		static constexpr uint64 MagicValue;
		static constexpr uint16 SupportedVersion = 0x0001;
		//static constexpr uint24 BlockMagicValue = 0x7D036E96;

		static constexpr uint32 BlockSize = 4096;

		static constexpr uint32 SegmentsPerFile = BlockSize / sizeof(uint32); // index table fits one block
		static constexpr uint32 SolutionsPerSegment = 16;
		static constexpr uint32 SegmentAlignment = 16;
	};

	struct SolutionsFileHeader
	{
		uint64 magic;
		uint16 version;
		uint64 workspaceId;
	};

	struct SolutionsFileIndexBlock
	{
		uint32 segmentsIndexTable[SolutionsFileConfig::SegmentsPerFile];
	};

	struct SolutionsFileBlockHeader
	{
		uint32 magic;
		uint32 length;
		uint32 checksum;
		struct
		{
			XTProblemId problemId;
			uint32 sourceEndOffset;
			XTTestingPolicy testingPolicy;
			XTLanguage language;
		} solutions[SolutionsFileConfig::SolutionsPerSegment];
	};

	// solutions temp file ==================================================================//

	struct SolutionsTempFileHeader
	{
		static constexpr uint24 MagicValue = 0xDA994C;
		static constexpr uint8 SupportedVersion = 0x01;

		uint24 magic;
		uint8 version;
	};

	struct SolutionsTempFileRecordHeader
	{
		uint32 sourceLength;
		uint32 checksum;
		XTProblemId solutionDesc;
		XTLanguage language;
		XTTestingPolicy testingPolicy;
	};
}
#pragma pack(pop, 1)

bool SolutionFiles::open()
{
	// index file
	{
		if (!indexFile.open("index", FileAccessMode::ReadWrite, FileOpenMode::OpenExisting))
		{
			localFailMessage = DbgMsgFmt("error opening solution index file"));
			goto label_localFail;
		}

		IndexFileHeader header;
		if (!indexFile.read(header))
		{
			Debug::Warning(DbgMsgFmt("error reading index file"));
			return false;
		}
		if (header.magic != IndexFileConfig::MagicValue)
		{
			Debug::Warning(DbgMsgFmt("invalid index file magic"));
			return false;
		}
		if (header.version != IndexFileConfig::SupportedVersion)
		{
			Debug::Warning(DbgMsgFmt("unsupported index file version"));
			return false;
		}
		workspaceId = header.workspaceId;
	}

	{

	}

	return true;
}