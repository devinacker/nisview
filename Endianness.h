#ifndef ENDIANNESS_H
#define ENDIANNESS_H

#include <QtEndian>

template<typename T, bool big = false>
struct Endian
{
	char m_val[sizeof(T)];

	Endian<T, big>& operator = (const T& val)
	{
		if (big)
			qToBigEndian(val, m_val);
		else
			qToLittleEndian(val, m_val);

		return *this;
	}

	operator T() const
	{
		T val = *reinterpret_cast<const T*>(m_val);

		if (big)
			return qFromBigEndian(val);
		else
			return qFromLittleEndian(val);
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

