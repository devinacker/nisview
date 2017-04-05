#include "formats/CompressNIS.h"
#include "Endianness.h"
#include <cstring>

#include <QDir>

#define packed __attribute__((__packed__))

//-----------------------------------------------------------------------------
namespace NISCompressLZS
{
	struct packed Header
	{
		char     magic[4];
		uint32le unpackedSize;
		uint32le packedSize;
		uint32le marker;
	};
}

//-----------------------------------------------------------------------------
bool NISCompressLZS::load(FileChunk &chunk)
{
	// https://disgaea.rustedlogic.net/LZS_format

	Header header;
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
	return chunk.makeChunk("0", data);
}

//-----------------------------------------------------------------------------
namespace NISCompressIMY
{
	struct packed Header
	{
		char     magic[4];
		uint16le unknown2;
		uint16le unknown3;
		uint16le offset;
		uint8    type;
		uint8    unknown6;
		uint16le unknown7;
		uint16le unknown8;
		uint32le padding[4];
		uint16le numInfoBytes;
	};
}

//-----------------------------------------------------------------------------
bool NISCompressIMY::load(FileChunk &chunk)
{
	// https://disgaea.rustedlogic.net/IMY_format
	// TODO: this is still kinda messed up (see some Phantom Brave map textures)

	Header header;
	chunk.readStruct(&header);
	if (std::memcmp(header.magic, "IMY\0", 4)
			|| header.type != 0x10)
	{
		return false;
	}

	const int lut[4] =
	{
		2,
		header.offset,
		header.offset + 2,
		header.offset - 2
	};

	QByteArray data;
	data.reserve(header.offset * header.unknown7); // ?

	QByteArray info;
	info.resize(header.numInfoBytes);
	chunk.read(info.data(), header.numInfoBytes);

	QByteArray src = chunk.readAll();

	int dataOffset = 0;
	for (auto byte : info)
	{
		if (data.size() == header.offset * header.unknown7)
			break;

		if ((byte & 0xc0) == 0xc0)
		{
			// copy from output
			int index = (byte >> 4) & 3;
			int size = (byte & 0xf) + 1;

			for (int i = 0; i < size; i++)
			{
				int from = data.size() - lut[index];
				if (from < 0 || lut[index] < 0)
					return false;

				data.append(data[from]);
				data.append(data[from+1]);
			}
		}
		else if (byte & 0xF0)
		{
			// copy from previous input
			int from = 2 * (byte - 16) + 2;
			if (from > dataOffset)
				return false;
			data.append(src[dataOffset - from]);
			data.append(src[dataOffset - from + 1]);
		}
		else
		{
			// read uncompressed data from input
			int size = 2 * (byte + 1);
			if (dataOffset + size > src.size())
				return false;
			data.append(src.data() + dataOffset, size);
			dataOffset += size;
		}
	}

	return chunk.makeChunk("0", data);
}
