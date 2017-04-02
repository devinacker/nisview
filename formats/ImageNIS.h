#ifndef IMAGENIS_H
#define IMAGENIS_H

#include "FileChunk.h"

namespace ImageTX2
{
	// test the size of a TX2 file because Disgaea texture packs
	// don't store offsets/sizes of individual texture files.
	// returns 0 if the file is not valid
	qint64 size(FileChunk&);

	bool load(FileChunk&);
}

#endif // IMAGENIS_H

