#include "cellular_automata.h"

#include <cmath>

CelAuto::Point::Point(int _x, int _y) :
	x(_x),
	y(_y) {}
CelAuto::Cell::Cell() :
	strength(0),
	label(0),
	checked(false) {}

CelAuto::ColourVec::ColourVec(int r, int g, int b) :
	red(r), green(g), blue(b) {}
CelAuto::ColourVec CelAuto::ColourVec::operator - (const ColourVec &second)
{
	return ColourVec(this->red - second.red,
					 this->green - second.green,
					 this->blue - second.blue);
}
CelAuto::Norm::Norm(int _height = 0, int _width = 0) :
	height(_height),
	width(_width)
{
	coef.resize(height*width);
}
double CelAuto::Norm::take(int x, int y,
						   int x_n, int y_n)
{
	double answer;
	int a = x_n - x,
		b = y_n - y;
	if(a == 1 && b == -1)
		answer = coef[x + y*width].n[0];
	if(a == 1 && b == 0)
		answer = coef[x + y*width].n[1];
	if(a == 1 && b == 1)
		answer = coef[x + y*width].n[2];
	if(a == 0 && b == 1)
		answer = coef[x + y*width].n[3];

	if(a == 0 && b == -1)
		answer = coef[x + (y-1)*width].n[3];
	if(a == -1 && b == -1)
		answer = coef[x-1 + (y-1)*width].n[2];
	if(a == -1 && b == 0)
		answer = coef[x-1 + y*width].n[1];
	if(a == -1 && b == 1)
		answer = coef[x-1 + (y+1)*width].n[0];
	return answer;
}

double CelAuto::metricL2(const ColourVec &vec)
{
	return sqrt( pow(vec.red, 2) + pow(vec.green, 2) + pow(vec.blue, 2) );
}
double CelAuto::monotonousFunction(double x)
{
	return 1 - (double)x/lumaMax;
}

CelAuto::CelAuto(const int& _height, const int& _width,
				 std::vector<ColourVec> &_image) :
	height(_height),
	width(_width),
	Image(_image)
{
	std::lock_guard<std::mutex> lock(dataMutex);
	is_programmGenerate = false;

	data.resize(width*height);
	swap_data = data;

	normales = Norm(height, width);

	set_number_of_threads();

	find_lumaMax();
	set_normales();
}

void CelAuto::set_number_of_threads()
{
	number_threads = std::thread::hardware_concurrency();
	if(number_threads < 1)
		number_threads = 1;
}
void CelAuto::find_lumaMax()
{
	std::vector<std::thread> vec_threads;
	lumaMax = 0;
	for(int i = 1; i <= number_threads; i++)
		vec_threads.push_back(std::thread(th_find_lumaMax, this, i, std::ref(lumaMax)));

	for(auto &it : vec_threads)
		it.join();
}
void CelAuto::th_find_lumaMax(const int num, double& max)
{
	int step_threads = height/number_threads;
	int start = (num-1) * step_threads,
			end = num * step_threads;
	if(num == number_threads)
		end = height;
	for(int y = start; y < end; y++)
		for(int x = 0; x < width; x++) {
			double pixelLuma = metricL2( Image[x + y*width] );
			if(pixelLuma > max)
				max = pixelLuma;
		}
}

void CelAuto::set_normales()
{
	std::vector<std::thread> vec_threads;
	for(int i = 1; i <= number_threads; i++)
		vec_threads.push_back(std::thread(th_set_normales, this, i));

	for(auto &it : vec_threads)
		it.join();
}
void CelAuto::th_set_normales(const int num)
{
	int step_threads = height/number_threads;
	int start = (num-1) * step_threads,
			end = num * step_threads;
	if(num == number_threads)
		end = height;
	for(int y = start; y < end; y++)
		for(int x = 0; x < width; x++) {
			double value;
			if(x != width-1 && y != 0) {
				value = monotonousFunction( metricL2( Image[x + y*width] -
												Image[(x+1) + (y-1)*width] ) );
				set_safe_norm(x, y, 0, value);		// 1 -1
			}
			if(x != width-1) {
				value = monotonousFunction( metricL2( Image[x + y*width] -
												Image[(x+1) + y*width] ) );
				set_safe_norm(x, y, 1, value);		// 1 0
			}
			if(x != width-1 && y != height-1) {
				value = monotonousFunction( metricL2( Image[x + y*width] -
													Image[(x+1) + (y+1)*width] ) );
				set_safe_norm(x, y, 2, value);		// 1 1
			}
			if(y != height-1) {
				value = monotonousFunction( metricL2( Image[x + y*width] -
												Image[x + (y+1)*width] ) );
				set_safe_norm(x, y, 3, value);		// 0 1
			}
		}
}
void CelAuto::set_safe_norm(const int x, const int y,
							const int num,
							const double val)
{
	std::lock_guard<std::mutex> lock(for_normales);
	normales.coef[x + y*normales.width].n[num] = val;
}



std::vector<CelAuto::Point> CelAuto::findNeighbors_Moore(const Point current, const bool swap)
{
	std::vector<Point> neighbors;

	bool left(true),
			top(true),
			right(true),
			bottom(true);

	int x = current.x,
		y = current.y;
	double strength = data[x + y*width].strength;

	if(swap && strength == 1)
		return neighbors;

	if(x == 0)
		left = false;
	if(x == width - 1)
		right = false;
	if(y == 0)
		top = false;
	if(y == height - 1)
		bottom = false;

	if(right) {
		if(swap) {
			if(data[x+1 + y*width].strength > strength)
				neighbors.push_back(Point(x+1, y));
		}
		else {
			if(!swap_data[x+1 + y*width].checked &&
					data[x+1 + y*width].strength < strength)
				neighbors.push_back(Point(x+1, y));
		}							// 001
		if(top)	{														// 0c1
			if(swap) {
				if(data[x+1 + (y-1)*width].strength > strength)
					neighbors.push_back(Point(x+1, y-1));
			}
			else {
				if(!swap_data[x+1 + (y-1)*width].checked &&
						data[x+1 + (y-1)*width].strength < strength)
					neighbors.push_back(Point(x+1, y-1));
			}
		}// 001
		if(bottom) {
			if(swap) {
				if(data[x+1 + (y+1)*width].strength > strength)
					neighbors.push_back(Point(x+1, y+1));
			}
			else {
				if(!swap_data[x+1 + (y+1)*width].checked &&
						data[x+1 + (y+1)*width].strength < strength)
					neighbors.push_back(Point(x+1, y+1));
			}
		}
	}
	if(left) {
		if(swap) {
			if(data[x-1 + y*width].strength > strength)
				neighbors.push_back(Point(x-1, y));
		}
		else {
			if(!swap_data[x-1 + y*width].checked &&
					data[x-1 + y*width].strength < strength)
				neighbors.push_back(Point(x-1, y));
		}							// 100
		if(top)	{															// 1c0
			if(swap) {
				if(data[x-1 + (y-1)*width].strength > strength)
					neighbors.push_back(Point(x-1, (y-1)));
			}
			else {
				if(!swap_data[x-1 + (y-1)*width].checked &&
						data[x-1 + (y-1)*width].strength < strength)
					neighbors.push_back(Point(x-1, (y-1)));
			}
		}// 100
		if(bottom) {
			if(swap) {
				if(data[x-1 + (y+1)*width].strength > strength)
					neighbors.push_back(Point(x-1, y+1));
			}
			else {
				if(!swap_data[x-1 + (y+1)*width].checked &&
						data[x-1 + (y+1)*width].strength < strength)
					neighbors.push_back(Point(x-1, y+1));
			}
		}
	}
	if(top) {
		if(swap) {
			if(data[x + (y-1)*width].strength > strength)
				neighbors.push_back(Point(x, y-1));
		}
		else {
			if(!swap_data[x + (y-1)*width].checked &&
					data[x + (y-1)*width].strength < strength)
				neighbors.push_back(Point(x, y-1));
		}
	}// 010
	if(bottom) {																// 0c0
		if(swap) {
			if(data[x + (y+1)*width].strength > strength)
				neighbors.push_back(Point(x, y+1));
		}
		else {
			if(!swap_data[x + (y+1)*width].checked &&
					data[x + (y+1)*width].strength < strength)
				neighbors.push_back(Point(x, y+1));
		}
	}// 010

	return neighbors;
}



void CelAuto::start()
{
	std::lock_guard<std::mutex> lock(dataMutex);
	if(!pixel_changes.empty() || is_programmGenerate) {
		is_programmGenerate = true;

		for(auto it : pixel_changes)
			data[it.x + it.y*width] = swap_data[it.x + it.y*width];
	}
}
void CelAuto::setCell(const int &x, const int &y, const int &_colour, const double &_strength)
{
	std::lock_guard<std::mutex> lock(prog_Generate);
	swap_data[x + y*width].label = _colour;
	swap_data[x + y*width].strength = _strength;
	swap_data[x + y*width].checked = true;

	pixel_changes.push_back(Point(x, y));
}

void CelAuto::generation()
{
	std::lock_guard<std::mutex> lock(prog_Generate);

	std::vector< std::vector<Point> > vec_pixel_threads;
	vec_pixel_threads.resize(number_threads);

	{
		std::lock_guard<std::mutex> lock(dataMutex);

		pixel_for_draw.insert(pixel_for_draw.end(), pixel_changes.begin(), pixel_changes.end());
		for(auto &it : swap_data)
			it.checked = false;

		int size = pixel_changes.size();
		for(int i = 0; i < size; i++)
			vec_pixel_threads[i % number_threads].push_back(pixel_changes[i]);
		pixel_changes.clear();
	}

	std::vector<std::thread> vec_threads;
	for(int i = 1; i <= number_threads; i++)
		vec_threads.push_back(std::thread(th_generation, this, vec_pixel_threads[i-1]));

	for(auto &it : vec_threads)
		it.join();

	setting_new_cells();
}
void CelAuto::th_generation(const std::vector<Point>& pixels)
{
	std::vector<Point> need_check;

	for(auto it : pixels) {
		std::lock_guard<std::mutex> lock(dataMutex);
        std::vector<Point> find = findNeighbors_Moore/*Neumann*/(it, false);
		for(auto it0 : find) {
			swap_data[it0.x + it0.y*width].checked = true;
            data[it0.x + it0.y*width].checked = false;
			need_check.push_back(it0);
		}
	}

	for(auto it : need_check)
    {
        Cell new_pixel = pixelVirus(it.x, it.y);
		if(new_pixel.checked)
            set_change(it.x, it.y, new_pixel);
	}
}
void CelAuto::set_change(const int &x, const int &y, Cell &dat)
{
    std::lock_guard<std::mutex> lock(dataMutex);

    if(data[x + y*width].checked)
        pixel_changes.push_back(Point(x, y));

	swap_data[x + y*width] = dat;
}
CelAuto::Cell CelAuto::pixelVirus(const int x, const int y)
{
	Cell answer = data[x + y*width];
	answer.checked = false;

    std::vector<Point> temp_vec = findNeighbors_Moore/*Neumann*/(Point(x, y), true);
	for (auto it : temp_vec ) {
        Cell temp = data[it.x + it.y*width];

        if(!data[x + y*width].checked && temp.label != answer.label) {
            std::lock_guard<std::mutex> lock(dataMutex);
            data[x + y*width].checked = true;
        }

		double coef = normales.take(x, y, it.x, it.y);

		if(coef*temp.strength > answer/*data[x + y*width]*/.strength) {
			answer.label = temp.label;
			answer.strength = coef*temp.strength;
			answer.checked = true;
            //break;
		}
	}
	return answer;
}
std::vector<CelAuto::Point> CelAuto::findNeighbors_Neumann(const Point current, const bool clear)
{
	std::vector<Point> neighbors;

	int x = current.x,
		y = current.y;
	double strength = data[x + y*width].strength;

	if(clear && strength == 1)
		return neighbors;

	if(x != 0) {
		if(clear) {
            if(data[x-1 + y*width].strength > strength)
				neighbors.push_back(Point(x-1, y));
		}
		else {
            if(!swap_data[x-1 + y*width].checked &&
                    data[x-1 + y*width].strength < strength)
				neighbors.push_back(Point(x-1, y));
		}
	}
	if(x != width - 1) {
		if(clear) {
            if(data[x+1 + y*width].strength > strength)
				neighbors.push_back(Point(x+1, y));
		}
		else {
            if(!swap_data[x+1 + y*width].checked &&
                    data[x+1 + y*width].strength < strength)
				neighbors.push_back(Point(x+1, y));
		}
	}
	if(y != 0) {
		if(clear) {
            if(data[x + (y-1)*width].strength > strength)
				neighbors.push_back(Point(x, y-1));
		}
		else {
            if(!swap_data[x + (y-1)*width].checked &&
                    data[x + (y-1)*width].strength < strength)
				neighbors.push_back(Point(x, y-1));
		}
	}
	if(y != height - 1) {
		if(clear) {
            if(data[x + (y+1)*width].strength > strength)
				neighbors.push_back(Point(x, y+1));
		}
		else {
            if(!swap_data[x + (y+1)*width].checked &&
                    data[x + (y+1)*width].strength < strength)
				neighbors.push_back(Point(x, y+1));
		}
	}

	return neighbors;
}
void CelAuto::setting_new_cells()
{
	std::lock_guard<std::mutex> lock(dataMutex);
	if(pixel_changes.empty())
		is_programmGenerate = false;
	else {
		for(auto it : pixel_changes) {
			if(data[it.x + it.y*width].label != swap_data[it.x + it.y*width].label)
				pixel_for_draw.push_back(it);
			data[it.x + it.y*width] = swap_data[it.x + it.y*width];
		}
	}
}


void CelAuto::stop()
{
	std::lock_guard<std::mutex> lock(dataMutex);
	is_programmGenerate = false;
}
