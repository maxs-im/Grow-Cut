#ifndef PAINTING_H
#define PAINTING_H

#include <QWidget>

#include <vector>
#include "cellular_automata.h"

QT_BEGIN_NAMESPACE
class QScrollArea;
class QImage;
class QPainter;
class QTimer;
QT_END_NAMESPACE

/*class CelAuto{
	public:
		struct ColourVec;
		struct Cell;
		struct Point;
		struct Norm;
};*/

class Painting : public QWidget
{
	Q_OBJECT

	public:
		Painting(QWidget *parent = 0);
        ~Painting();

		CelAuto *base;

		double strengthCell;
		bool originalPage;					//what imagine is displayed in the application
											//					(with stripes or original)


		bool openImage(const QString &);
		bool saveImage(const QString &);
		void setBrushColor(const int &);
		bool openBackGroundImage(const QString &);
		void setBackGround();
		//void startAlgorithm();
        void do_alg();
        //void stopAlgorithm();
		void reset();
		void cancel();

		QSize sizeHint() const override;
	private:

        QScrollArea *scrollBar;

		QImage workImage;
		QImage originalImage;
		QImage generationImage;
		QImage backImage;
		std::vector<CelAuto::ColourVec> image_under_CelAuto;

		int brush_paints_colour;
		bool is_Draw;						//is mouse drawing now
		bool is_Generating_image;
		bool imagine_with_BackGround;
		QPoint prev_point;
		int shift_vertical,
			shift_horizontal;

		void th_start();
		void check_borders(int &, int &);
		void convert_to_algorithm(const QImage &);
		void setImage(const QImage &);
		void repaint_glob(std::vector<CelAuto::Point>);
		void setGenerationImage();
	public slots:
		void startAlgorithm();
        void stopAlgorithm();
	signals:
        void started_alg();
        void finished();
	protected:
		void paintEvent(QPaintEvent *) override;
		void mouseMoveEvent(QMouseEvent *) override;
		void mousePressEvent(QMouseEvent *) override;
		void mouseReleaseEvent(QMouseEvent *) override;
};

#endif // PAINTING_H
