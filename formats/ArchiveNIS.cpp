#include "formats/ArchiveNIS.h"
#include "Endianness.h"
#include <cstring>

#include <QDir>
#include <qdebug.h>

#define packed __attribute__((__packed__))

//-----------------------------------------------------------------------------
template<class fileType>
static bool loadNISArchive(FileChunk &chunk, uint numFiles)
{
	for (uint i = 0; i < numFiles; i++)
	{
		fileType file;

		chunk.read(reinterpret_cast<char*>(&file), sizeof(file));
		file.name[sizeof(file.name) - 1] = '\0';

		if (!chunk.makeChunk(file.name, file.offset, file.size))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool ArchivePSPFS::load(FileChunk &chunk)
{
	struct packed PSPFSHeader
	{
		char     magic[8];
		uint32le numFiles;
		uint32le dummy;
	} header;

	struct packed PSPFSFile
	{
		char     name[44];
		uint32le size;
		uint32le offset;
	};

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "PSPFS_V1", 8))
	{
		return loadNISArchive<PSPFSFile>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::load(FileChunk &chunk)
{
	struct packed NISPackHeader
	{
		char     magic[8];
		uint32le dummy;
		uint32le numFiles;
	} header;

	struct packed NISPackFile
	{
		char     name[32];
		uint32le offset;
		uint32le size;
		uint32le dummy;
	};

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "NISPACK\0", 8))
	{
		return loadNISArchive<NISPackFile>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ArchiveDSARC::load(FileChunk &chunk)
{
	struct packed DSARCHeader
	{
		char     magic[8];
		uint32le numFiles;
		uint32le dummy;
	} header;

	struct packed DSARCFile
	{
		char     name[40];
		uint32le size;
		uint32le offset;
	};

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "DSARC FL", 8))
	{
		return loadNISArchive<DSARCFile>(chunk, header.numFiles);
	}
	else if (!std::memcmp(header.magic, "DSARCIDX", 8))
	{
		// Eventually when rewriting these, we'll probably care about the file #s here
		// but just skip them for now
		chunk.seek(chunk.pos() + 2 * ((header.numFiles + 1) & ~1));

		return loadNISArchive<DSARCFile>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
bool ArchiveNISPS2::load(FileChunk &chunk)
{
	// Load a Disgaea 1/2/etc. PS2 archive (DATA.DAT + SECTOR.H)
	// Assumes that DATA.DAT is not inside another archive (which it should never be)

	if (!chunk.name().endsWith("/DATA.DAT"))
	{
		return false;
	}

	QDir dir = QFileInfo(chunk.name()).dir();
	QFile sector(dir.filePath("SECTOR.H"));
	if (!sector.open(QFile::ReadOnly))
	{
		return false;
	}

	struct packed PS2File
	{
		char     name[32];
		uint32le offset;
		uint32le size;
	} file;

	while (!sector.atEnd())
	{
		sector.read(reinterpret_cast<char*>(&file), sizeof(file));
		file.name[sizeof(file.name) - 1] = '\0';

		if (!file.name[0])
		{
			break;
		}

		if (!chunk.makeChunk(file.name, file.offset * 2048, file.size))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool ArchiveLZS::load(FileChunk &)
{
	// https://disgaea.rustedlogic.net/LZS_format

	struct packed LZSHeader
	{
		char     magic[4];
		uint32le packedSize;
		uint32le unpackedSize;
		uint32le marker;
	} header;

	return false;
}

//-----------------------------------------------------------------------------
bool ArchiveLZS::loadRaw(FileChunk &)
{
	return false;
}
