#include "formats/ArchiveNIS.h"
#include "formats/ImageNIS.h"
#include "Endianness.h"
#include <cstring>

#include <QDir>

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
bool NISArchivePS2::load(FileChunk &chunk)
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
bool NISArchiveGeneric::load(FileChunk &chunk)
{
	struct packed GenericHeader
	{
		uint32le numFiles;
		uint32le dummy[3];
	} header;

	struct packed GenericFile
	{
		uint32le endOffset;
		char     name[28];
	} file;

	chunk.readStruct(&header);
	// enforce a reasonable(?) limit on number of files to avoid false positives
	if (header.numFiles == 0 || header.numFiles >= 1<<16
			|| header.dummy[0] || header.dummy[1] || header.dummy[2])
	{
		return false;
	}

	uint startOffset = sizeof(header) + header.numFiles * sizeof(file);
	uint lastOffset = 0;
	for (uint i = 0; i < header.numFiles; i++)
	{
		chunk.readStruct(&file);
		file.name[sizeof(file.name) - 1] = '\0';

		// validate file name (ASCII only)
		for (uint j = 0; j < sizeof(file.name); j++)
		{
			char c = file.name[j];
			if ((c > 0 && c < 0x20) || c > 0x7e)
			{
				return false;
			}
		}

		if (file.endOffset < lastOffset || file.endOffset + startOffset > chunk.size())
		{
			return false;
		}

		uint size = file.endOffset - lastOffset;
		if (!chunk.makeChunk(file.name, lastOffset + startOffset, size))
		{
			return false;
		}
		lastOffset = file.endOffset;
	}

	return true;
}

//-----------------------------------------------------------------------------
bool NISArchiveGeneric::loadIndexed(FileChunk &chunk)
{
	struct packed GenericHeader
	{
		uint32le numFiles;
		uint32le unknown;
	} header;

	chunk.readStruct(&header);
	// enforce a reasonable(?) limit on number of files to avoid false positives
	if (header.numFiles == 0 || header.numFiles >= 1<<16
			|| header.unknown != 0x20000)
	{
		return false;
	}

	for (uint i = 0; i < header.numFiles; i++)
	{
		chunk.seek(4*i + sizeof(header));

		uint offset = chunk.readLE<quint32>();
		int size = -1;
		if (i < header.numFiles - 1)
		{
			size = chunk.readLE<quint32>() - offset;
		}

		if (!chunk.makeChunk(QString::number(i), offset, size))
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
bool NISArchiveTexture::load(FileChunk &chunk)
{
	struct TexHeader
	{
		uint32le numFiles;
		uint32le unknown;
		uint32le padding[2];
	} header;

	chunk.readStruct(&header);

	if (header.numFiles == 0 || header.numFiles >= 0x10000
			|| header.padding[0] || header.padding[1])
	{
		return false;
	}

	quint64 offset = (16 * header.numFiles) + sizeof(header);
	for (uint i = 0; i < header.numFiles; i++)
	{
		char nameBuf[16];
		chunk.seek((16 * i) + sizeof(header));
		chunk.read(nameBuf, 16);
		QString name = nameBuf; // assume null-terminated?

		// should (probably) contain tx2 files only
		// (at least i've only encountered these so far)
		if (!name.endsWith(".tx2", Qt::CaseInsensitive))
		{
			return false;
		}

		chunk.makeChunk(name, 0, 0);
		/* TODO
		chunk.seek(offset);
		qint64 size = ImageTX2::size(chunk);
		if (size <= 0)
		{
			return false;
		}
		if (!chunk.makeChunk(name, offset, size))
		{
			return false;
		}

		offset += size;
		*/
	}

	return true;
}
