#include "formats/ImageNIS.h"
#include "Endianness.h"

#include <QImage>

#define packed __attribute__((__packed__))

namespace ImageTX2
{
	enum
	{
		FormatDXT1     = 0,
		FormatDXT5     = 2,
		FormatBGRA     = 3,
		Format4BPP     = 16,
		Format4BPP_RGBA,
		Format8BPP     = 256,
		Format8BPP_RGBA,
	};

	struct Header
	{
		uint16le width;
		uint16le height;
		uint16le colors;	// see above enum
		uint16le unknown1;
		uint16le paletteSize;
		uint16le numPalettes;
		uint16le unknown2;
		uint16le flags;     // palette swizzling, possibly more (I forget which bits)
	};

	struct packed DXT1
	{
		uint16le color[2];
		uint8    colorLUT[4];
	};

	struct packed DXT3
	{
		uint8 alpha[2];
		uint8 alphaLUT[6];
		DXT1  color;
	};
}

bool ImageTX2::load(FileChunk &chunk)
{
	Header header;
	chunk.readStruct(&header);

	if (!header.width || !header.height)
		return false;
	if (header.colors >= Format4BPP && (!header.paletteSize || !header.numPalettes))
		return false;

	QImage::Format format;
	switch (header.colors)
	{
	case FormatDXT1:
	case FormatDXT5:
		// TODO
		return false;
	case FormatBGRA:
		format = QImage::Format_ARGB32;
		break;

	case Format4BPP:
	case Format4BPP_RGBA:
	case Format8BPP:
	case Format8BPP_RGBA:
		format = QImage::Format_Indexed8;
		break;

	default:
		// others not supported yet or invalid
		return false;
	}

	QImage image(header.width, header.height, format);

	// TODO: load all palettes if available, add option to select
	QVector<QRgb> palette;
	palette.resize(header.paletteSize);

	for (int i = 0; i < palette.size(); i++)
	{
		int color = i;

		if (header.flags == 0
				&& (header.colors == Format8BPP || header.colors == Format8BPP_RGBA))
		{
			color = (i & 0xE7) | ((i & 8) << 1) | ((i & 16) >> 1);
		}

		QRgb rgb = chunk.readLE<quint32>();
		if (header.colors == Format4BPP_RGBA || header.colors == Format8BPP_RGBA)
		{
			//TODO swap red and blue channels here
		}

		// TODO: make sure alpha value is correct for images with transparency
		if (rgb & 0x80000000)
			rgb |= 0xFF000000;

		palette[color] = rgb;
	}
	image.setColorTable(palette);

	// skip other palettes for now, if any
	chunk.seek(sizeof(header) + (header.numPalettes * header.paletteSize * 4));

	// load pixel data
	uchar *bits = image.bits();

	switch (header.colors)
	{
	case FormatBGRA:
	case Format8BPP:
	case Format8BPP_RGBA:
		chunk.read(reinterpret_cast<char*>(bits), image.byteCount());
		break;

	case Format4BPP:
	case Format4BPP_RGBA:
		for (int i = 0; i < image.byteCount(); i += 2)
		{
			uchar byte = chunk.readByte();
			bits[i]   = byte & 0xF;
			bits[i+1] = byte >> 4;
		}
		break;

	default:
		break;
	}

	chunk.setValue(image);
	return true;
}
