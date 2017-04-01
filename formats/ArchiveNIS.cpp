#include "formats/ArchiveNIS.h"
#include <cstring>

//-----------------------------------------------------------------------------
bool ArchivePSPFS::test(FileChunk &chunk)
{
	char magic[8];
	chunk.reset();
	int size = chunk.read(magic, sizeof(magic));

	return size == sizeof(magic)
		   && !std::memcmp(magic, "PSPFS_V1", sizeof(magic));
}

//-----------------------------------------------------------------------------
bool ArchivePSPFS::load(FileChunk &chunk)
{
	chunk.seek(8);
	auto numFiles = chunk.readLE<quint32>();

	chunk.seek(16);
	for (uint i = 0; i < numFiles; i++)
	{
		char name[44];
		quint32 size, offset;

		chunk.read(name, sizeof(name));
		name[sizeof(name) - 1] = '\0';

		size = chunk.readLE<quint32>();
		offset = chunk.readLE<quint32>();

		if (!chunk.makeChunk(name, offset, size))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::test(FileChunk &chunk)
{
	char magic[8];
	chunk.reset();
	int size = chunk.read(magic, sizeof(magic));

	return size == sizeof(magic)
		   && !std::memcmp(magic, "NISPACK\x00", sizeof(magic));
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::load(FileChunk &chunk)
{
	chunk.seek(12);
	auto numFiles = chunk.readLE<quint32>();

	for (uint i = 0; i < numFiles; i++)
	{
		char name[32];
		quint32 size, offset;

		chunk.read(name, sizeof(name));
		name[sizeof(name) - 1] = '\0';

		offset = chunk.readLE<quint32>();
		size = chunk.readLE<quint32>();
		// dummy
		chunk.readLE<quint32>();

		if (!chunk.makeChunk(name, offset, size))
		{
			return false;
		}
	}

	return true;
}
