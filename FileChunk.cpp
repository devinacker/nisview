#include "FileChunk.h"
#include "FormatSpec.h"

//-----------------------------------------------------------------------------
FileChunk::FileChunk(QIODevice *parent, const QString& name, qint64 offset, qint64 size)
	: QIODevice(parent)
	, m_name(name)
	, m_pParent(parent)
	, m_offset(offset)
	, m_size(size)
	, m_pos(parent->pos() - offset)
{
	if (size < 0)
	{
		m_size = parent->size() - offset;
	}

	loadChunks();
}

//-----------------------------------------------------------------------------
void FileChunk::loadChunks()
{
	if (m_pParent->isOpen())
	{
		clearChunks();

		// allow underlying QFile etc. device to handle buffering (for now)
		// so that seeking and reading w/ this instance works as expected
		QIODevice::open(ReadOnly | Unbuffered);

		qint64 oldpos = m_pParent->pos();
		m_pFormat = FormatSpec::match(*this);
		m_type = m_pFormat->m_name;
		m_pFormat->load(*this);
		m_pParent->seek(oldpos);
	}
}

//-----------------------------------------------------------------------------
void FileChunk::clearChunks()
{
	// signal recursively clears child chunks
	QIODevice::close();

	m_children.clear();
}

//-----------------------------------------------------------------------------
bool FileChunk::makeChunk(const QString &name, qint64 offset, qint64 size)
{
	if (offset + size <= m_size)
	{
		m_children.append(new FileChunk(this, name, offset, size));

		return true;
	}

	return false;
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
qint64 FileChunk::readData(char *data, qint64 maxlen)
{
	if (m_pos < 0 || m_pos >= m_size)
		return -1;

	m_pParent->seek(m_offset + m_pos);
	qint64 read = m_pParent->read(data, qMin(maxlen, m_size - m_pos));

	m_pos = this->pos();
	return read;
}

//-----------------------------------------------------------------------------
qint64 FileChunk::readLineData(char *data, qint64 maxlen)
{
	if (m_pos < 0 || m_pos >= m_size)
		return -1;

	m_pParent->seek(m_offset + m_pos);
	qint64 read = m_pParent->readLine(data, qMin(maxlen, m_size - m_pos));

	m_pos = this->pos();
	return read;
}

//-----------------------------------------------------------------------------
qint64 FileChunk::writeData(const char*, qint64)
{
	setErrorString(tr("FileChunk is read-only"));
	return -1;
}
