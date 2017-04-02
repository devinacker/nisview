#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <FileChunk.h>
#include <FileChunkModel.h>

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>

//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
	, m_pChunks(0)
	, m_pChunkModel(new FileChunkModel)
{
	ui->setupUi(this);

	ui->treeView->setModel(m_pChunkModel);
	ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
			this, SLOT(chunkContextMenu(QPoint)));

	connect(ui->actionOpen_Archive, SIGNAL(triggered(bool)), this, SLOT(openArchive()));

	connect(ui->actionSave_Chunk_As, SIGNAL(triggered(bool)), this, SLOT(saveChunkToFile()));
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
}

//-----------------------------------------------------------------------------
void MainWindow::chunkContextMenu(const QPoint &pos)
{
	ui->menuChunk->exec(ui->treeView->mapToGlobal(pos));
}

//-----------------------------------------------------------------------------
void MainWindow::openArchive()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open Archive"), m_lastFileName);
	if (!fileName.isEmpty())
	{
		m_lastFileName = fileName;

		m_pChunkModel->setChunks(0);
		delete m_pChunks;
		m_pChunks = 0;

		m_file.close();
		m_file.setFileName(fileName);
		if (m_file.open(QFile::ReadOnly))
		{
			m_pChunks = new FileChunk(&m_file, fileName);
			m_pChunkModel->setChunks(m_pChunks);
		}
		else
		{
			QMessageBox::critical(this, tr("Open Archive"), tr("Unable to open %1.").arg(fileName));
		}
	}
}

//-----------------------------------------------------------------------------
void MainWindow::saveChunkToFile()
{
	// get currently selected chunk
	QModelIndexList indexes = ui->treeView->selectionModel()->selectedRows();
	for (QModelIndex& index : indexes)
	{
		FileChunk *chunk = reinterpret_cast<FileChunk*>(index.internalPointer());
		if (!chunk) continue;

		QString fileName = QFileDialog::getSaveFileName(this, tr("Save Chunk to File"), chunk->name());
		if (!fileName.isEmpty())
		{
			QFile file(fileName);
			if (file.open(QFile::WriteOnly))
			{
				chunk->reset();
				QByteArray data = chunk->readAll();

				file.write(data);
				file.close();
			}
			else
			{
				QMessageBox::critical(this, tr("Open Archive"), tr("Unable to open %1.").arg(fileName));
			}
		}
		else
		{
			// user cancelled save
			break;
		}
	}
}
