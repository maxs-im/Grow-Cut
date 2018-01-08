#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "painting.h"

#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QLabel>
#include <QImage>

#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    paintBlock(new Painting),
    th_work(new QThread)
{
    ui->setupUi(this);

    setCentralWidget(paintBlock);

    paintBlock->moveToThread(th_work);

    connect(paintBlock, SIGNAL(started_alg()), th_work, SLOT(start()));
    connect(th_work, SIGNAL(started()), paintBlock, SLOT(startAlgorithm()));
    connect(paintBlock, SIGNAL(finished()), th_work, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    paintBlock->stopAlgorithm();
    th_work->wait();
    delete th_work;
    delete paintBlock;

    delete ui;
}

void MainWindow::on_actionOpen_triggered()
{
    const QString filePath =
			QFileDialog::getOpenFileName(this,
										 tr("Open File"),
										 QDir::currentPath(),
										 tr("Image Files (*.png *.jpg *.bmp *.ico)"));

    //const QString filePath = "F:\\Solutions\\Labs\\Lab_3(5)\\For Test.jpg";
	if(!filePath.isEmpty()) {
		if (!paintBlock->openImage(filePath)) {
			QMessageBox::information(this,
									 tr("Error"),
									 tr("Can not <b>load</b> this file"));
			return;
        }
	}
    statusBar()->showMessage(filePath);
}

void MainWindow::on_actionSave_As_triggered()
{
	const QString pathToSave = QDir::currentPath() + "/untitled.png";
	const QString filename = QFileDialog::getSaveFileName(this,
														  tr("Save As"),
														  pathToSave,
														  tr("(*.png)"));
	if(!filename.isEmpty())
		if(!paintBlock->saveImage(filename))
			QMessageBox::information(this,
									 tr("Error"),
									 tr("Can not <b>save</b> this file"));
}

void MainWindow::on_actionAbout_triggered()
{
	QMessageBox::about(this, tr("Interesting?"),
					   tr("      With this program <b>GrowCut</b><br>"
						  "You can allocate objects from the picture."));
}

void MainWindow::on_actionBackGround_triggered()
{
	ui->actionBackGround->setChecked(true);
	ui->actionFrontGround->setChecked(false);
	paintBlock->setBrushColor(1);
}

void MainWindow::on_actionFrontGround_triggered()
{
	ui->actionBackGround->setChecked(false);
	ui->actionFrontGround->setChecked(true);
	paintBlock->setBrushColor(2);
}

void MainWindow::on_actionStart_triggered()
{
    paintBlock->stopAlgorithm();
    th_work->wait();

    paintBlock->do_alg();
}

void MainWindow::on_actionFinish_triggered()
{
	paintBlock->stopAlgorithm();
}

void MainWindow::on_actionCancel_triggered()
{
	paintBlock->cancel();
}

void MainWindow::on_actionDelete_triggered()
{
	paintBlock->reset();
}

void MainWindow::on_actionAdd_new_Object_on_BackGround_triggered()
{
	const QString filePath =
			QFileDialog::getOpenFileName(this,
										 tr("Select BackGround Image"),
										 QDir::currentPath(),
										 tr("Image Files (*.png *.jpg *.bmp *.ico)"));

	if(!filePath.isEmpty()) {
		if (!paintBlock->openBackGroundImage(filePath)) {
			QMessageBox::information(this,
									 tr("Error"),
									 tr("Can not <b>load</b> this file"));
			return;
		}
	}
}

void MainWindow::on_actionSet_BackGround_triggered()
{
	paintBlock->setBackGround();
	paintBlock->adjustSize();
}
