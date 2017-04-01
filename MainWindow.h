#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>

#include <QString>

class FileChunk;
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

private:
	Ui::MainWindow *ui;

	QString m_lastFileName;
	QFile m_file;
	FileChunk *m_pChunks;
	FileChunkModel *m_pChunkModel;
};

#endif // MAINWINDOW_H
