#include "formats/CompressNIS.h"
#include "Endianness.h"
#include <cstring>

#include <QDir>

#define packed __attribute__((__packed__))

//-----------------------------------------------------------------------------
bool NISCompressLZS::load(FileChunk &chunk)
{
	// https://disgaea.rustedlogic.net/LZS_format

	struct packed LZSHeader
	{
		char     magic[4];
		uint32le unpackedSize;
		uint32le packedSize;
		uint32le marker;
	} header;

	chunk.readStruct(&header);
	if (std::memcmp(header.magic, "dat\0", 4)
			|| header.unpackedSize < header.packedSize
			|| header.packedSize + 4 != chunk.size()
			// TODO: D2 PS2's start.lzs seems to use a larger value here
			|| header.marker > 0xFF)
	{
		return false;
	}

	QByteArray data;
	data.reserve(header.unpackedSize);

	QByteArray src = chunk.readAll();

	for (int i = 0; i < src.size();)
	{
		quint8 byte = src[i++];
		if (byte == header.marker)
		{
			if (i == src.size()) return false;

			quint8 dist = src[i++];
			if (dist == byte) // escaped marker
			{
				data.append(byte);
			}
			else
			{
				if (i == src.size()) return false;

				quint8 count = src[i++];
				if (dist > header.marker) dist--;
				if (dist > data.size()) return false; // invalid distance

				for (uint j = 0; j < count; j++)
				{
					data.append(data[data.size() - dist]);
				}
			}
		}
		else
		{
			data.append(byte);
		}
	}

	if (data.size() != static_cast<int>(header.unpackedSize))
	{
		return false;
	}

	// create a single unnamed chunk representing the decompressed contents
	// (in case this contains something other than the start.dat-style archive)
	return chunk.makeChunk(QObject::tr("(decompressed data)"), data);
}
