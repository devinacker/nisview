#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "FormatSpec.h"
#include "FileChunk.h"
#include "FileChunkModel.h"

#include <QFileDialog>
#include <QFile>
#include <QLabel>
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

	connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(displayChunk(QModelIndex)));
	connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
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

//-----------------------------------------------------------------------------
void MainWindow::displayChunk(const QModelIndex &index)
{
	FileChunk *chunk = reinterpret_cast<FileChunk*>(index.internalPointer());
	if (!chunk) return;

	int tabIndex = -1;

	if (m_openTabs.contains(chunk))
	{
		tabIndex = m_openTabs[chunk];
	}
	// Just display image chunks for now, as a test.
	// More interesting stuff comes later...
	else switch (chunk->type())
	{
	case FormatSpec::Image:
	{
		QLabel *label = new QLabel();
		label->setPixmap(QPixmap::fromImage(chunk->value<QImage>()));

		tabIndex = ui->tabWidget->addTab(label, chunk->name());
	}
		break;

	default:
		break;
	}

	if (tabIndex >= 0)
	{
		ui->tabWidget->setCurrentIndex(tabIndex);

		m_openTabs[chunk] = tabIndex;
	}
}

//-----------------------------------------------------------------------------
void MainWindow::closeTab(int index)
{
	m_openTabs.remove(m_openTabs.key(index));

	QWidget *widget = ui->tabWidget->widget(index);
	ui->tabWidget->removeTab(index);
	delete widget;
}
