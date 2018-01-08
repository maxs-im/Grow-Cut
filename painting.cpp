		#include "painting.h"
	#include "mainwindow.h"

	#include <QPainter>
	#include <QMouseEvent>
    #include <QScrollArea>

	#include <QMessageBox>
    #include <QTime>


	Painting::Painting(QWidget *parent) :
        QWidget(parent),
        scrollBar(new QScrollArea)
	{
        scrollBar->setBackgroundRole(QPalette::Dark);
        scrollBar->setWidget(this);

		strengthCell = 1;
		originalPage = true;
		brush_paints_colour = 1;
		is_Draw = false;
		is_Generating_image = false;
		imagine_with_BackGround = false;

		shift_horizontal = 0;
		shift_vertical = 0;
			//set default background
		backImage = QImage(1000, 1000, QImage::Format_RGB32);
		backImage.fill(Qt::GlobalColor::black);

		base = nullptr;
	}

    Painting::~Painting()
    {
        if(base)
            delete base;
    }

	bool Painting::openImage(const QString &filePath)
	{
		QImage image;
		if(!image.load(filePath))
			return false;

		setImage(image);
        update();
		return true;
	}

	bool Painting::saveImage(const QString &filePath)
	{
		if(imagine_with_BackGround)
			return backImage.save(filePath, "png");
		else
			return generationImage.save(filePath, "png");
	}

	void Painting::setImage(const QImage &_image)
	{
		originalImage = _image.convertToFormat(QImage::Format_RGB32);
		convert_to_algorithm(originalImage);

		setGenerationImage();

		base = new CelAuto(originalImage.height(), originalImage.width(),
						   image_under_CelAuto);
		workImage = originalImage;
	}

	void Painting::setBrushColor(const int &label_colour)
	{
		brush_paints_colour = label_colour;
	}

	bool Painting::openBackGroundImage(const QString &filePath)
	{
		QImage image;
		if(!image.load(filePath))
			return false;

		backImage = image.convertToFormat(QImage::Format_RGB32);
		return true;
	}

	void Painting::paintEvent(QPaintEvent */*event*/)
	{
		QPainter painter(this);

		if(is_Generating_image) {
			painter.drawImage(QPoint(0, 0), generationImage);
			return;
		}
		if(imagine_with_BackGround) {
			this->adjustSize();
			painter.drawImage(QPoint(0, 0), backImage);
			return;
		}
		if(originalPage) {
			painter.drawImage(QPoint(0, 0), workImage);
			return;
		}
		else {
			painter.drawImage(QPoint(0, 0), originalImage);
			return;
		}

	}

	QSize Painting::sizeHint() const
	{
		if(imagine_with_BackGround)
			return backImage.size();
		return workImage.size();
	}

	void Painting::mouseMoveEvent(QMouseEvent *event)
	{
		if(is_Draw) {
			int x = event->pos().x(),
				y = event->pos().y();
			check_borders(x, y);
			QPoint new_point = QPoint(x, y);

			QPainter painter(&workImage);
			QPen pen;
			switch(brush_paints_colour)
			{
				case 1:
					pen.setColor(Qt::blue);
					break;
				case 2:
					pen.setColor(Qt::red);
					break;
			}
			base->setCell(new_point.x(), new_point.y(), brush_paints_colour, strengthCell);

			pen.setWidth(7);
			pen.setStyle(Qt::SolidLine);
			pen.setCapStyle(Qt::RoundCap);

			painter.setPen(pen);

			painter.drawLine(prev_point, new_point);
			prev_point = new_point;
			update();
		}
	}
	void Painting::mousePressEvent(QMouseEvent *event)
	{
		is_Draw = true;
		int x = event->pos().x(),
			y = event->pos().y();
		check_borders(x, y);

		prev_point = QPoint(x, y);
	}
	void Painting::check_borders(int &x, int &y)
	{
		if(x < 0)
			x = 0;
		if(y < 0)
			y = 0;
		if(x > originalImage.width()-1)
			x = originalImage.width()-1;
		if(y > originalImage.height()-1)
			y = originalImage.height()-1;
	}

	void Painting::mouseReleaseEvent(QMouseEvent *)
	{
		is_Draw = false;
	}

    void Painting::do_alg()
    {
        emit started_alg();
    }

	void Painting::startAlgorithm()
	{

        if(originalImage.isNull() || !base ||
				base->is_programmGenerate) {
            if(base->is_programmGenerate)
                return;
            emit finished();
			return;
		}

        QTime timer(0, 0, 0, 0);
        timer.start();

		repaint();

		is_Generating_image = true;

		base->start();

		while(base->is_programmGenerate) {
			base->generation();
			repaint_glob(base->pixel_for_draw);
			base->pixel_for_draw.clear();
		}

        int time = timer.elapsed();

        int t_minutes = (time/1000)/60;
        QString result_time = QString::number( (double)(time - t_minutes*60)/1000 ) + "s";
        if(t_minutes > 0)
            result_time = QString::number(t_minutes) + "m : " + result_time;

        QString s_time = "<h1>Generations completed</h1><br>Time: " + result_time;

		QMessageBox::warning(this,
								 tr("OK"),
                                 s_time);
		is_Generating_image = false;
		workImage = generationImage;

        emit finished();
	}

	void Painting::stopAlgorithm()
	{
        if(originalImage.isNull()) {
            emit finished();
            return;
        }
		base->stop();

        emit finished();
	}
	void Painting::reset()
	{
		if(originalImage.isNull())
			return;

		delete base;

		base = new CelAuto(originalImage.height(), originalImage.width(),
						   image_under_CelAuto);
		workImage = originalImage;
		setGenerationImage();

		originalPage = true;
		is_Generating_image = false;
		imagine_with_BackGround = false;
		update();
	}
	void Painting::convert_to_algorithm(const QImage &i_image)
	{
		int image_H= i_image.height();
		int image_W = i_image.width();

		image_under_CelAuto.resize(image_W*image_H);
		for(int y = 0; y < image_H; y++)
			for(int x = 0; x < image_W; x++) {
				QColor i_pixel = i_image.pixelColor(QPoint(x, y));
				CelAuto::ColourVec pixel(i_pixel.red(),
										 i_pixel.green(),
										 i_pixel.blue());
				image_under_CelAuto[x + y*image_W] = pixel;
			}
	}
	void Painting::repaint_glob(std::vector<CelAuto::Point> data)
	{
		for(auto it : data) {
			int x = it.x,
				y = it.y;
			int label = base->data[x + y*base->width].label;

			if(label == 0) {
				QColor pixel = originalImage.pixel(x, y);
				pixel = pixel.darker(150);
				//generationImage.setPixelColor(QPoint(x, y), pixel);
				generationImage.setPixelColor(QPoint(x, y), QColor(0, 255, 0));
			}
			else if(label == 1)
				generationImage.setPixelColor(QPoint(x, y), QColor(0, 0, 0));
			else if(label == 2)
				generationImage.setPixel(QPoint(x, y), originalImage.pixel(x, y));
		}

        repaint();
	}

	void Painting::setBackGround()
	{
		if(generationImage.isNull())
			return;
		for(int x = 0; x < originalImage.width(); x++)
			for(int y = 0; y < originalImage.height(); y++) {
				int set_y = y + shift_vertical,
					set_x = x + shift_horizontal;
				if( !backImage.valid(set_x, set_y) || base->data[x + y*base->width].label != 2)
					continue;
				backImage.setPixelColor(QPoint(set_x, set_y), generationImage.pixel(x, y));
			}
		imagine_with_BackGround = true;

		update();
	}

	void Painting::cancel()
	{
		if(originalImage.isNull())
			return;

		imagine_with_BackGround = false;
		update();
	}

	void Painting::setGenerationImage()
	{
		generationImage = originalImage;
		for(int x = 0; x < originalImage.width(); x++)
			for(int y = 0; y < originalImage.height(); y++) {
				QColor pixel = originalImage.pixel(x, y);
				pixel = pixel.darker(150);
				generationImage.setPixelColor(QPoint(x, y), pixel);
				//generationImage.setPixelColor(QPoint(x, y), QColor(0, 255, 0));
			}
	}
