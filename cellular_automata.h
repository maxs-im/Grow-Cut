#ifndef CELLULAR_AUTOMATA_H
#define CELLULAR_AUTOMATA_H

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class CelAuto
{
	public:
		struct Point {
			int x;
			int y;
			Point(int _x = 0, int _y = 0);
		};

		struct ColourVec {
			short int red;
			short int green;
			short int blue;
			ColourVec(int r = 0, int g = 0, int b = 0);
			ColourVec operator- (const ColourVec &);
		};

		struct Cell {
			double strength;
			int label;
			bool checked;
			Cell();
		};
		struct Norm {
			private:
				int height;
			public:
				struct doubleX4{
					double n[4];
				};

				Norm(int, int);
				int	width;
				std::vector<doubleX4> coef;
				double take(int, int, int, int);
		};

		CelAuto(const int&, const int&, std::vector<ColourVec> &);

		int height;
		int width;
		Norm normales;
		std::vector<Cell> data;
		std::vector<Point> pixel_for_draw;
		volatile bool is_programmGenerate;

		void start();
		void stop();
		void generation();
		void setCell(const int&, const int&, const int&, const double&);
	private:
		std::vector<ColourVec> Image;
		std::vector<Cell> swap_data;
		std::vector<Point> pixel_changes;
		double lumaMax;
		int number_threads;

		std::mutex dataMutex;
		std::mutex prog_Generate;
		std::mutex for_normales;

		std::vector<Point> findNeighbors_Neumann(const Point, const bool);
		std::vector<Point> findNeighbors_Moore(const Point, const bool);
		double monotonousFunction(double val);
		double metricL2(const ColourVec &);
		void find_lumaMax();
		void th_find_lumaMax(const int, double&);
        Cell pixelVirus(const int, const int);
		void th_generation(const std::vector<Point> &);
        void set_change(const int &, const int &, Cell &);
		void th_synchronization();
		void setting_new_cells();
		void set_normales();
		void th_set_normales(const int);
		void set_safe_norm(const int, const int,
						   const int,
						   const double);

		void set_number_of_threads();
};

#endif // CELLULAR_AUTOMATA_H
