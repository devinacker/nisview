#include "formats/ImageNIS.h"
#include "Endianness.h"

namespace ImageTX2
{
	struct TX2Header
	{
		uint16le width;
		uint16le height;
		uint16le colors;
		uint16le unknown1;
		uint16le paletteSize;
		uint16le numPalettes;
		uint16le unknown2;
		uint16le flags;
	};
}

qint64 ImageTX2::size(FileChunk &chunk)
{
	TX2Header header;
	chunk.readStruct(&header);

	// TODO
	return -1;
}

bool ImageTX2::load(FileChunk &chunk)
{
	// temp
	return size(chunk) > 0;
}
