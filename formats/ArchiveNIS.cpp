#include "formats/ArchiveNIS.h"
#include <cstring>

#define packed __attribute__((__packed__))

//-----------------------------------------------------------------------------
template<size_t size>
static bool testNISHeader(FileChunk &chunk, const char (&magic)[size])
{
	qint64 insize = size - 1;

	char buf[insize];
	chunk.reset();
	int read = chunk.read(buf, insize);

	return read == insize && !std::memcmp(buf, magic, insize);
}

//-----------------------------------------------------------------------------
template<class headerType, class fileType>
static bool loadNISArchive(FileChunk &chunk)
{
	headerType header;

	chunk.reset();
	chunk.read(reinterpret_cast<char*>(&header), sizeof(header));
	uint numFiles = qFromLittleEndian(header.numFiles);

	for (uint i = 0; i < numFiles; i++)
	{
		fileType file;
		quint64 size, offset;

		chunk.read(reinterpret_cast<char*>(&file), sizeof(file));
		file.name[sizeof(file.name) - 1] = '\0';

		size = qFromLittleEndian(file.size);
		offset = qFromLittleEndian(file.offset);

		if (!chunk.makeChunk(file.name, offset, size))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool ArchivePSPFS::test(FileChunk &chunk)
{
	return testNISHeader(chunk, "PSPFS_V1");
}

//-----------------------------------------------------------------------------
bool ArchivePSPFS::load(FileChunk &chunk)
{
	struct packed PSPFSHeader
	{
		char    magic[8];
		quint32 numFiles;
		quint32 dummy;
	};

	struct packed PSPFSFile
	{
		char    name[44];
		quint32 size;
		quint32 offset;
	};

	return loadNISArchive<PSPFSHeader, PSPFSFile>(chunk);
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::test(FileChunk &chunk)
{
	return testNISHeader(chunk, "NISPACK\x00");
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::load(FileChunk &chunk)
{
	struct packed NISPackHeader
	{
		char    magic[8];
		quint32 dummy;
		quint32 numFiles;
	};

	struct packed NISPackFile
	{
		char    name[32];
		quint32 offset;
		quint32 size;
		quint32 dummy;
	};

	return loadNISArchive<NISPackHeader, NISPackFile>(chunk);
}

//-----------------------------------------------------------------------------
bool ArchiveDSARC::test(FileChunk &chunk)
{
	return testNISHeader(chunk, "DSARC FL");
}

//-----------------------------------------------------------------------------
bool ArchiveDSARC::load(FileChunk &chunk)
{
	struct packed DSARCHeader
	{
		char    magic[8];
		quint32 numFiles;
		quint32 dummy;
	};

	struct packed DSARCFile
	{
		char    name[40];
		quint32 size;
		quint32 offset;
	};

	return loadNISArchive<DSARCHeader, DSARCFile>(chunk);
}
