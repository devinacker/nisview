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

	connect(ui->actionOpen_Archive, SIGNAL(triggered(bool)), this, SLOT(openArchive()));
}

//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
	delete ui;
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
