#ifndef FORMATSPEC_H
#define FORMATSPEC_H

#include <QString>
#include <FileChunk.h>

typedef bool (*FormatFunc)(FileChunk&);

struct FormatSpec
{
	const QString m_name;

	enum FormatType
	{
		Unknown,
		Archive,
		Image,
		PackedFile,
	};
	const int m_type;

	FormatFunc load;

	FormatSpec(const char* name = "Unknown", int type = Unknown, FormatFunc load = 0);
	static const FormatSpec* match(FileChunk& chunk);
};

#endif // FORMATSPEC_H
