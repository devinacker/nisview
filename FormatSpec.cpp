#include "FormatSpec.h"
#include <qvector.h>

#include "formats/ArchiveNIS.h"
#include "formats/CompressNIS.h"

//-----------------------------------------------------------------------------
static bool defaultFunc(FileChunk&)
{
	return false;
}

static FormatSpec unknownFormat;

//-----------------------------------------------------------------------------
static QVector<FormatSpec> formats =
{
	// Archive formats
	FormatSpec("PSPFS archive", FormatSpec::Archive, ArchivePSPFS::load),
	FormatSpec("NISPack archive", FormatSpec::Archive, ArchiveNISPack::load),
	FormatSpec("DSARC archive", FormatSpec::Archive, ArchiveDSARC::load),
	FormatSpec("NIS PS2 archive", FormatSpec::Archive, NISArchivePS2::load),
	FormatSpec("NIS generic archive", FormatSpec::Archive, NISArchiveGeneric::load),

	// Compression formats
	FormatSpec("NIS LZS packed", FormatSpec::PackedFile, NISCompressLZS::load),
	FormatSpec("IMY chunk", FormatSpec::PackedFile, 0), // TODO

	// Image formats
	// TODO

	// Other formats
	// TODO
};

//-----------------------------------------------------------------------------
FormatSpec::FormatSpec(const char *typeName, int type, FormatFunc load)
	: m_name(QObject::tr(typeName))
	, m_type(type)
	, load(!!load ? load : defaultFunc)
{
}

//-----------------------------------------------------------------------------
const FormatSpec* FormatSpec::match(FileChunk& chunk)
{
	for (FormatSpec& format : formats)
	{
		chunk.reset();

		if (format.load(chunk))
		{
			return &format;
		}

		chunk.clearChunks();
	}

	return &unknownFormat;
}
