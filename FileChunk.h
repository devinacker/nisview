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
	~FileChunk() {}

	bool open(OpenMode) { return false; }
	void close() {}

	void loadChunks();
	void clearChunks();

	bool makeChunk(const QString& name, qint64 offset, qint64 size = -1);

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

	inline QString type() const
	{
		return m_type;
	}

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
	bool atEnd() const
	{
		return m_pos >= m_size || m_pParent->bytesAvailable();
	}

	qint64 bytesAvailable() const
	{
		return qMax(0ll, qMin(m_size - m_pos, m_pParent->bytesAvailable()));
	}

	qint64 bytesToWrite() const
	{
		return 0;
	}

	bool canReadLine() const
	{
		return m_size < m_offset && m_pParent->canReadLine();
	}

	qint64 pos() const
	{
		return m_pParent->pos() - m_offset;
	}

	bool reset()
	{
		return seek(0);
	}

	bool seek(qint64 pos)
	{
		if (m_pParent->seek(pos + m_offset))
		{
			m_pos = pos;
			return true;
		}

		m_pos = this->pos();
		return false;
	}

	qint64 size() const
	{
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
	QString m_type;

	QIODevice *m_pParent;
	qint64 m_offset;
	qint64 m_size;
	qint64 m_pos;
};

#endif // FILECHUNK_H
