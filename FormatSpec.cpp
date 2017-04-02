#include "FormatSpec.h"
#include <qvector.h>

#include "formats/ArchiveNIS.h"

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
	FormatSpec("NIS PS2 archive", FormatSpec::Archive, ArchiveNISPS2::load),

	FormatSpec("LZS archive (uncompressed)", FormatSpec::Archive, ArchiveLZS::loadRaw),
	FormatSpec("LZS archive", FormatSpec::Archive, ArchiveLZS::load),

	// Image formats
	// TODO

	// Other formats
	// TODO
	FormatSpec("IMY chunk", FormatSpec::PackedFile, 0),
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
