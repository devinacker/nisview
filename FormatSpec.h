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
	};
	const int m_type;

	FormatFunc test;
	FormatFunc load;

	FormatSpec(const char* name = "Unknown", int type = Unknown, FormatFunc test = 0, FormatFunc load = 0);
	static const FormatSpec* match(FileChunk& device);
};

#endif // FORMATSPEC_H
