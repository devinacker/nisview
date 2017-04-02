#include "FileChunk.h"
#include "FormatSpec.h"

#include <cstring>

#include <qdebug.h>

//-----------------------------------------------------------------------------
FileChunk::FileChunk(QIODevice *parent, const QString& name, qint64 offset, qint64 size)
	: QIODevice(parent)
	, m_name(name)
	, m_pParent(parent)
	, m_offset(offset)
	, m_size(size)
{
	if (size < 0)
	{
		m_size = parent->size() - offset;
	}

	loadChunks();
}

//-----------------------------------------------------------------------------
FileChunk::FileChunk(QIODevice *parent, const QString& name, const QByteArray& data)
	: QIODevice(parent)
	, m_name(name)
	, m_pParent(parent)
	, m_offset(0)
	, m_size(data.size())
{
	this->setValue(data);

	loadChunks();
}

//-----------------------------------------------------------------------------
void FileChunk::loadChunks()
{
	if (m_pParent->isOpen())
	{
		// allow underlying QFile etc. device to handle buffering (for now)
		// so that seeking and reading w/ this instance works as expected
		QIODevice::open(ReadOnly | Unbuffered);

		qint64 oldpos = m_pParent->pos();
		m_pFormat = FormatSpec::match(*this);
		m_type = m_pFormat->m_name;
		m_pParent->seek(oldpos);
	}
}

//-----------------------------------------------------------------------------
void FileChunk::clearChunks()
{
	for (FileChunk *chunk : m_children)
	{
		delete chunk;
	}

	m_children.clear();
}

//-----------------------------------------------------------------------------
bool FileChunk::makeChunk(const QString &name, qint64 offset, qint64 size)
{
	if (offset + size <= this->size())
	{
		m_children.append(new FileChunk(this, name, offset, size));

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
bool FileChunk::makeChunk(const QString& name, const QByteArray &data)
{
	m_children.append(new FileChunk(this, name, data));
	return true;
}

//-----------------------------------------------------------------------------
FileChunk* FileChunk::getChunk(const QString &name) const
{
	QString myChildName = name.section('/', 0, 1);
	QString subChildName = name.section('/', 1);

	for (FileChunk* child : m_children)
	{
		if (child->name() == myChildName)
		{
			if (subChildName.isEmpty())
			{
				return child->getChunk(subChildName);
			}

			return child;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------
FileChunk* FileChunk::getChunk(int index) const
{
	if (index < m_children.size())
	{
		return m_children[index];
	}

	return 0;
}

//-----------------------------------------------------------------------------
QString FileChunk::fullPath() const
{
	QString path;
	QString parentClass = m_pParent->metaObject()->className();

	if (parentClass == "FileChunk")
	{
		path = qobject_cast<FileChunk*>(m_pParent)->fullPath() + "/";
	}

	return path + m_name;
}

//-----------------------------------------------------------------------------
qint64 FileChunk::readData(char *data, qint64 maxlen)
{
	qint64 read;
	qint64 pos = this->pos();

	maxlen = qMin(maxlen, this->size() - pos);
	if (pos < 0 || maxlen < 0)
		return -1;

	if (m_value.type() == QVariant::ByteArray)
	{
		// reading from decompressed data
		std::memcpy(data, m_value.toByteArray().constData() + pos, maxlen);
		read = maxlen;
	}
	else
	{
		// reading from parent file
		m_pParent->seek(m_offset + pos);
		read = m_pParent->read(data, maxlen);
	}

	return read;
}

//-----------------------------------------------------------------------------
qint64 FileChunk::readLineData(char *data, qint64 maxlen)
{
	qint64 pos = this->pos();

	if (pos < 0 || pos >= this->size())
		return -1;

	m_pParent->seek(m_offset + pos);
	qint64 read = m_pParent->readLine(data, qMin(maxlen, this->size() - pos));

	return read;
}

//-----------------------------------------------------------------------------
qint64 FileChunk::writeData(const char*, qint64)
{
	setErrorString(tr("FileChunk is read-only"));
	return -1;
}
