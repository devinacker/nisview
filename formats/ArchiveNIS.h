#ifndef ARCHIVENIS_H
#define ARCHIVENIS_H

#include "FileChunk.h"

namespace ArchivePSPFS
{
	bool load(FileChunk&);
}

namespace ArchiveNISPack
{
	bool load(FileChunk&);
}

namespace ArchiveDSARC
{
	bool load(FileChunk&);
}

namespace ArchiveNISPS2
{
	bool load(FileChunk&);
}

namespace ArchiveLZS
{
	bool load(FileChunk&);
	bool loadRaw(FileChunk&);
}

#endif // ARCHIVENIS_H
