#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QLabel;
class QThread;
QT_END_NAMESPACE

class Painting;

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
		void on_actionOpen_triggered();
		void on_actionSave_As_triggered();
		void on_actionAbout_triggered();
		void on_actionCancel_triggered();
		void on_actionDelete_triggered();
		void on_actionStart_triggered();
		void on_actionBackGround_triggered();
		void on_actionFrontGround_triggered();
		void on_actionFinish_triggered();

		void on_actionAdd_new_Object_on_BackGround_triggered();

		void on_actionSet_BackGround_triggered();

	private:
		Ui::MainWindow *ui;
		Painting *paintBlock;

        QThread *th_work;
};

#endif // MAINWINDOW_H
