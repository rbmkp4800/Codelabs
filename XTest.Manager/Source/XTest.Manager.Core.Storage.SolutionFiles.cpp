#include <XLib.Debug.h>

#include "XTest.Manager.Core.Storage.SolutionFiles.h"

using namespace XLib;
using namespace XTest;
using namespace XTest::Manager::_Core::_Storage;

#pragma pack(push, 1)
namespace
{
	// TODO: remove from here
	constexpr uint32 MagicFromChars32(char c0, char c1, char c2, char c3)
		{ return uint32(c0) | (uint32(c1) << 8) | (uint32(c1) << 16) | (uint32(c1) << 24); }

	// index file ===========================================================================//
	//               index file structure
	// +--------+------+---------------+---------------+----
	// | header | 0000 |   records 0   |   records 1   |   ...
	// +--------+------+---------------+---------------+----
	//  \_____________/ \_____________/
	//      block 0         block 1
	//
	//           index file records block structure
	// +----------+----------+---------+----------+-----+-------+
	// | record 0 | record 1 |   ...   | record N | 000 | CRC32 |
	// +----------+----------+---------+----------+-----+-------+
	//  \__________________block - 4096 bytes__________________/
	//                  (511 records per block)

	struct IndexFileHeader
	{
		uint32 magic;
		uint16 version;
		uint64 workspaceId;
	};

	struct IndexFileRecord
	{
		XTProblemId problemId;
		uint32 index;
	};

	class IndexFileConfig abstract final
	{
	public:
		static constexpr uint32 MagicValue = MagicFromChars32('X', 'T', 'I', 'F');
		static constexpr uint16 SupportedVersion = 0x0001;

		static constexpr uint32 BlockSize = 4096;
		static constexpr uint32 RecordsPerBlock =
			(BlockSize - sizeof(uint32)) / sizeof(IndexFileRecord);
	};

	// solutions file =======================================================================//
	//                 solutions file structure
	// +--------+----+-------------+-----------------+-----------+----
	// | header | 00 | index table |    segment 1    | segment 2 |  ...
	// +--------+----+-------------+-----------------+-----------+----
	// \________4096 bytes________/ \_______________/ \_________/   ...
	//        (index block)              M bytes        N bytes 
	//
	//  * index table entry holds the length of corresponding segment
	//    to get absolute index, first need to take prefix sum
	// TODO: add checksum to index block
	//
	//
	// TODO:       solutions file segment structure
	//                             ...

	struct SolutionsFileHeader
	{
		uint32 magic;
		uint16 version;
		uint64 workspaceId;
	};

	class SolutionsFileConfig abstract final
	{
	public:
		static constexpr uint32 MagicValue = MagicFromChars32('X', 'T', 'S', 'F');
		static constexpr uint16 SupportedVersion = 0x0001;
		//static constexpr uint16 BlockMagicValue = 0x7D036E96;

		using IndexType = uint16;
		static constexpr uint32 IndexBlockSize = 4096;
		static constexpr uint32 SegmentsPerFile =
			(IndexBlockSize - sizeof(SolutionsFileHeader)) / sizeof(IndexType);

		static constexpr uint32 SolutionsPerSegment = 16;
		static constexpr uint32 SegmentAlignment = 32;
	};

	struct SolutionsFileIndexBlock
	{
		SolutionsFileConfig::IndexType segmentsIndexTable[SolutionsFileConfig::SegmentsPerFile];
	};

	struct SolutionsFileSegmentHeader
	{
		uint16 magic;
		SolutionsFileConfig::IndexType segmentLength;
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

	// TODO:

	/*struct SolutionsTempFileHeader
	{
		static constexpr uint32 MagicValue = 0xDA994C;
		static constexpr uint8 SupportedVersion = 0x01;

		uint32 magic;
		uint8 version;
	};

	struct SolutionsTempFileRecordHeader
	{
		uint32 sourceLength;
		uint32 checksum;
		XTProblemId solutionDesc;
		XTLanguage language;
		XTTestingPolicy testingPolicy;
	};*/

	/*struct IndexTempFileBlockHeader
	{
		uint32 magic;
		uint8 version;
		uint32 globalBlockIndex;
	};

	struct IndexTempFileRecord
	{
		IndexFileRecord record;
		uint32 checksum;
	};*/
}
#pragma pack(pop, 1)

bool SolutionFiles::open()
{
	char *errorMessage = nullptr;

	// index file
	{
		if (!indexFile.open("index", FileAccessMode::ReadWrite, FileOpenMode::OpenExisting))
		{
			errorMessage = DbgMsgFmt("error opening index file");
			goto label_error;
		}

		IndexFileHeader header;
		if (!indexFile.read(header))
		{
			errorMessage = DbgMsgFmt("error reading index file");
			goto label_error;
		}
		if (header.magic != IndexFileConfig::MagicValue)
		{
			errorMessage = DbgMsgFmt("invalid index file magic");
			goto label_error;
		}
		if (header.version != IndexFileConfig::SupportedVersion)
		{
			errorMessage = DbgMsgFmt("unsupported index file version");
			goto label_error;
		}
		if (indexFile.getSize() % IndexFileConfig::BlockSize != 0)
		{
			errorMessage = DbgMsgFmt("invalid index file size");
			goto label_error;
		}
		workspaceId = header.workspaceId;
	}

label_error:
	indexFile.close();

	workspaceId = 0;

	return true;
}