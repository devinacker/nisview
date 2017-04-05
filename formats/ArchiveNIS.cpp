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
namespace ArchivePSPFS
{
	struct packed Header
	{
		char     magic[8];
		uint32le numFiles;
		uint32le dummy;
	};

	struct packed File
	{
		char     name[44];
		uint32le size;
		uint32le offset;
	};
}

//-----------------------------------------------------------------------------
bool ArchivePSPFS::load(FileChunk &chunk)
{
	Header header;

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "PSPFS_V1", 8))
	{
		return loadNISArchive<File>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
namespace ArchiveNISPack
{
	struct packed Header
	{
		char     magic[8];
		uint32le dummy;
		uint32le numFiles;
	};

	struct packed File
	{
		char     name[32];
		uint32le offset;
		uint32le size;
		uint32le dummy;
	};
}

//-----------------------------------------------------------------------------
bool ArchiveNISPack::load(FileChunk &chunk)
{
	Header header;

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "NISPACK\0", 8))
	{
		return loadNISArchive<File>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
namespace ArchiveDSARC
{
	struct packed Header
	{
		char     magic[8];
		uint32le numFiles;
		uint32le dummy;
	};

	struct packed File
	{
		char     name[40];
		uint32le size;
		uint32le offset;
	};
}

//-----------------------------------------------------------------------------
bool ArchiveDSARC::load(FileChunk &chunk)
{
	Header header;

	chunk.readStruct(&header);
	if (!std::memcmp(header.magic, "DSARC FL", 8))
	{
		return loadNISArchive<File>(chunk, header.numFiles);
	}
	else if (!std::memcmp(header.magic, "DSARCIDX", 8))
	{
		// Eventually when rewriting these, we'll probably care about the file #s here
		// but just skip them for now
		chunk.seek(chunk.pos() + 2 * ((header.numFiles + 1) & ~1));

		return loadNISArchive<File>(chunk, header.numFiles);
	}

	return false;
}

//-----------------------------------------------------------------------------
namespace NISArchivePS2
{
	struct packed File
	{
		char     name[32];
		uint32le offset;
		uint32le size;
	};
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

	while (!sector.atEnd())
	{
		File file;

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
namespace NISArchiveGeneric
{
	struct packed Header
	{
		uint32le numFiles;
		uint32le dummy[3];
	};

	struct packed File
	{
		uint32le endOffset;
		char     name[28];
	};

	struct packed IndexedHeader
	{
		uint32le numFiles;
		uint32le unknown;
	};
}

//-----------------------------------------------------------------------------
bool NISArchiveGeneric::load(FileChunk &chunk)
{
	Header header;
	chunk.readStruct(&header);
	// enforce a reasonable(?) limit on number of files to avoid false positives
	if (header.numFiles == 0 || header.numFiles >= 1<<16
			|| header.dummy[0] || header.dummy[1] || header.dummy[2])
	{
		return false;
	}

	uint startOffset = sizeof(header) + header.numFiles * sizeof(File);
	uint lastOffset = 0;
	for (uint i = 0; i < header.numFiles; i++)
	{
		File file;
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
	IndexedHeader header;
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
namespace NISArchiveTexture
{
	struct packed Header
	{
		uint32le numFiles;
		uint32le unknown1; // texture format? (probably always <256)
		uint32le unknown2; // always 0?
		uint32le unknown3; // either 0 or between numFiles and unknown1?
						   // (probably always <256)
	};

	struct packed File
	{
		uint32le size;
		uint32le padding[3];
	};
}

//-----------------------------------------------------------------------------
bool NISArchiveTexture::load(FileChunk &chunk)
{
	Header header;
	chunk.readStruct(&header);

	if (header.numFiles == 0 || header.numFiles >= 1<<8
			|| header.unknown1 >= 1<<8 || header.unknown3 >= 1<<8)
	{
		return false;
	}

	quint64 offset = (16 * header.numFiles) + sizeof(header);
	for (uint i = 0; i < header.numFiles; i++)
	{
		File file;

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

		chunk.seek(offset);
		chunk.readStruct(&file);
		if (!chunk.makeChunk(name, chunk.pos(), file.size))
		{
			return false;
		}

		offset += file.size + sizeof(file);
	}

	return true;
}
