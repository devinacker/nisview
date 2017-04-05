#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QString>

#include "FileChunk.h"

class FileChunkModel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void openArchive();
	void saveChunkToFile();

	void chunkContextMenu(const QPoint&);
	void displayChunk(const QModelIndex&);
	void closeTab(int);

private:
	Ui::MainWindow *ui;

	QString m_lastFileName;
	QFile m_file;
	FileChunk *m_pChunks;
	FileChunkModel *m_pChunkModel;
	QMap<FileChunk*, int> m_openTabs;
};

#endif // MAINWINDOW_H
