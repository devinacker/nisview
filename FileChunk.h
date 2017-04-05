#ifndef FILECHUNK_H
#define FILECHUNK_H

#include <QIODevice>
#include <QVariant>
#include <QtEndian>

struct FormatSpec;

class FileChunk : public QIODevice
{
	Q_OBJECT

public:
	FileChunk() = delete;
	FileChunk(QObject * parent) = delete;

	FileChunk(QIODevice *parent, const QString& name, qint64 offset = 0, qint64 size = -1);
	FileChunk(QIODevice *parent, const QString& name, const QByteArray& data);
	~FileChunk()
	{
		clearChunks();
	}

	bool open(OpenMode) { return false; }
	void close() {}

	void loadChunks();
	void clearChunks();

	bool makeChunk(const QString& name, qint64 offset, qint64 size = -1);
	bool makeChunk(const QString& name, const QByteArray &data);

	FileChunk* parentChunk() const
	{
		return qobject_cast<FileChunk*>(m_pParent);
	}

	FileChunk* getChunk(const QString& name) const;
	FileChunk* getChunk(int index) const;

	inline QString name() const
	{
		return m_name;
	}

	QString fullPath() const;

	inline int numChunks() const
	{
		return m_children.size();
	}

	inline int indexOfChild(FileChunk *child)
	{
		return m_children.indexOf(child);
	}

	inline int chunkIndex()
	{
		if (parentChunk())
		{
			return parentChunk()->indexOfChild(this);
		}

		return -1;
	}

	QString format() const;
	int type() const;

	template<typename T>
	T value() const
	{
		return m_value.value<T>();
	}

	template<typename T>
	bool setValue(const T& value)
	{
		// TODO: make sure the chunk supports the type of data being passed in
		m_value.setValue<T>(value);
		return true;
	}

	quint8 readByte()
	{
		quint8 value;
		read(reinterpret_cast<char*>(&value), 1);
		return value;
	}

	template<class T>
	qint64 readStruct(T* data)
	{
		return read(reinterpret_cast<char*>(data), sizeof(T));
	}

	template<typename T>
	T readLE()
	{
		T value;
		read(reinterpret_cast<char*>(&value), sizeof(T));
		return qFromLittleEndian<T>(value);
	}

	template<typename T>
	T readBE()
	{
		T value;
		read(reinterpret_cast<char*>(&value), sizeof(T));
		return qFromBigEndian<T>(value);
	}

	/*
	 * Reimplementation of QIODevice public methods
	 */
	qint64 bytesToWrite() const
	{
		return 0;
	}

	bool canReadLine() const
	{
		return size() < m_offset && m_pParent->canReadLine();
	}

	qint64 size() const
	{
		if (m_value.type() == QVariant::ByteArray)
		{
			// reading from decompressed data
			return m_value.toByteArray().size();
		}

		// reading from parent file
		return qMin(m_size, m_pParent->size());
	}

protected:
	/*
	 * Reimplementation of QIODevice protected methods
	 */
	qint64 readData(char *data, qint64 maxlen);
	qint64 readLineData(char *data, qint64 maxlen);
	qint64 writeData(const char*, qint64);

private:
	const FormatSpec *m_pFormat;

	QList<FileChunk*> m_children;
	QVariant m_value;

	QString m_name;
	QIODevice *m_pParent;
	qint64 m_offset;
	qint64 m_size;
};

#endif // FILECHUNK_H
