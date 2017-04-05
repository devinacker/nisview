#include "FileChunkModel.h"

//-----------------------------------------------------------------------------
FileChunkModel::FileChunkModel(QObject *parent)
	: QAbstractItemModel(parent)
	, m_pChunks(0)
{
}

//-----------------------------------------------------------------------------
int FileChunkModel::rowCount(const QModelIndex &parent) const
{
	auto *chunk = reinterpret_cast<FileChunk*>(parent.internalPointer());
	// any child chunks or only the parent chunk/file
	// (TODO: support multiple open files at once)
	if (chunk)
	{
		return chunk->numChunks();
	}
	else if (!parent.isValid() && m_pChunks)
	{
		// root item(?)
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------
QVariant FileChunkModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
	{
		switch (section)
		{
		case ColumnName:
			return tr("Name");

		case ColumnType:
			return tr("Type");

		case ColumnSize:
			return tr("Size");
		}
	}
	else if (orientation == Qt::Vertical && role == Qt::DisplayRole)
	{
		// row number
		return section + 1;
	}

	return QAbstractItemModel::headerData(section, orientation, role);
}

//-----------------------------------------------------------------------------
QVariant FileChunkModel::data(const QModelIndex &index, int role) const
{
	auto *chunk = reinterpret_cast<FileChunk*>(index.internalPointer());
	if (!chunk) return QVariant();

	switch (index.column())
	{
	case ColumnName:
		if (role == Qt::DisplayRole)
			return chunk->name();
		break;

	case ColumnType:
		if (role == Qt::DisplayRole)
			return chunk->format();
		break;

	case ColumnSize:
		if (role == Qt::DisplayRole)
			return chunk->size(); // TODO: make this look better
		break;
	}

	return QVariant();
}

//-----------------------------------------------------------------------------
QModelIndex FileChunkModel::index(int row, int column, const QModelIndex &parent) const
{
	auto *chunk = reinterpret_cast<FileChunk*>(parent.internalPointer());
	if (!parent.isValid())
	{
		return this->createIndex(row, column, m_pChunks);
	}
	else if (chunk && row < chunk->numChunks())
	{
		return this->createIndex(row, column, chunk->getChunk(row));
	}

	return QModelIndex();
}

//-----------------------------------------------------------------------------
QModelIndex FileChunkModel::parent(const QModelIndex &child) const
{
	auto *chunk = reinterpret_cast<FileChunk*>(child.internalPointer());
	if (chunk)
	{
		return this->createIndex(chunk->chunkIndex(), 0, chunk->parentChunk());
	}

	return QModelIndex();
}

//-----------------------------------------------------------------------------
void FileChunkModel::setChunks(FileChunk *chunks)
{
	beginResetModel();
	m_pChunks = chunks;
	endResetModel();
}
