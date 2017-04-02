#ifndef COMPRESSNIS_H
#define COMPRESSNIS_H

#include "FileChunk.h"

namespace NISCompressLZS
{
	bool load(FileChunk&);
}

namespace NISCompressIMY
{
	bool load(FileChunk&);
}

#endif // COMPRESSNIS_H

