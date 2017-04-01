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
	FormatSpec("PSPFS archive", FormatSpec::Archive, ArchivePSPFS::test, ArchivePSPFS::load),
	FormatSpec("NISPack archive", FormatSpec::Archive, ArchiveNISPack::test, ArchiveNISPack::load),

	// Image formats
	// TODO

	// Other formats
	// TODO
};

//-----------------------------------------------------------------------------
FormatSpec::FormatSpec(const char *typeName, int type, FormatFunc test, FormatFunc load)
	: m_name(QObject::tr(typeName))
	, m_type(type)
	, test(!!test ? test : defaultFunc)
	, load(!!load ? load : defaultFunc)
{
}

//-----------------------------------------------------------------------------
const FormatSpec* FormatSpec::match(FileChunk& device)
{
	for (FormatSpec& format : formats)
	{
		if (format.test(device))
		{
			return &format;
		}
	}

	return &unknownFormat;
}
