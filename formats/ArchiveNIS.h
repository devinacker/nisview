#ifndef ARCHIVENIS_H
#define ARCHIVENIS_H

#include "FileChunk.h"

namespace ArchivePSPFS
{
	bool test(FileChunk&);
	bool load(FileChunk&);
}

namespace ArchiveNISPack
{
	bool test(FileChunk&);
	bool load(FileChunk&);
}

namespace ArchiveDSARC
{
	bool test(FileChunk&);
	bool load(FileChunk&);
}

#endif // ARCHIVENIS_H
