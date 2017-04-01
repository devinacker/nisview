#ifndef FILECHUNKMODEL_H
#define FILECHUNKMODEL_H

#include <QAbstractItemModel>
#include "FileChunk.h"

class FileChunkModel : public QAbstractItemModel
{
public:
	enum
	{
		ColumnName,
		ColumnType,
		ColumnSize,
		ColumnCount
	};

	FileChunkModel(QObject *parent = 0);

	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &) const
	{
		return ColumnCount;
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	QVariant data(const QModelIndex &index, int role) const;
	QModelIndex index(int row, int column, const QModelIndex &parent) const;
	QModelIndex parent(const QModelIndex &child) const;

	void setChunks(FileChunk*);

private:
	FileChunk *m_pChunks;
};

#endif // FILECHUNKMODEL_H
