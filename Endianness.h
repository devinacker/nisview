#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#include <QtEndian>

template<typename T, bool big = false>
struct Endian
{
	T m_val;

	Endian<T, big>& operator = (const T& val)
	{
		if (big)
			m_val = qToBigEndian(val);
		else
			m_val = qToLittleEndian(val);

		return *this;
	}

	operator T() const
	{
		if (big)
			return qFromBigEndian(m_val);
		else
			return qFromLittleEndian(m_val);
	}
};

typedef qint8  int8;
typedef quint8 uint8;

typedef Endian<qint16>  int16le;
typedef Endian<quint16> uint16le;
typedef Endian<qint32>  int32le;
typedef Endian<quint32> uint32le;

typedef Endian<qint16, true>  int16be;
typedef Endian<quint16, true> uint16be;
typedef Endian<qint32, true>  int32be;
typedef Endian<quint32, true> uint32be;

#endif // ENDIANNESS_H

