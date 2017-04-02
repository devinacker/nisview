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

namespace NISArchivePS2
{
	bool load(FileChunk&);
}

namespace NISArchiveGeneric
{
	bool load(FileChunk&);
}

#endif // ARCHIVENIS_H
