#pragma once

#include <math.h>
#include <vector>
#include <iomanip>
#include <cctype>

const double PI = 3.14159265358979323846264338327950288;

#ifndef _SDL_stdinc_h
#include <stdint.h>
typedef int8_t Sint8;
typedef uint8_t Uint8;
typedef int16_t Sint16;
typedef uint16_t Uint16;
typedef int32_t Sint32;
typedef uint32_t Uint32;
typedef int64_t Sint64;
typedef uint64_t Uint64;
#endif

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#include <algorithm>

int pnRandi(int range){
	if (range == 0){
		return(0);
	} else {
		return(range - (rand() % (range * 2 + 1)));
	}
}
double dRand(double range){
	return(((rand() % RAND_MAX) / (double)(RAND_MAX - 1) * range));
}
int randi(int range){
	return(rand() % range);
}
double pnRand(double range){
	return(range * (1 - ((rand() % RAND_MAX) / (double)(RAND_MAX - 1) * 2)));
}
double roundTo(double val, double size){
	return(round(val / size) * size);
}
int roundTo(int val, int size){
	return(int((val + (size / 2)) / size) * size);
}

int sign(double val){
	if (val < 0){
		return(-1);
	} else if (val == 0){
		return(0);
	} else {
		return(1);
	}
}

int wrapAround(int x, unsigned int range){
	if (x < 0){
		return(range - 1 - (-(x + 1) % range));
	} else {
		return(x % range);
	}
}

double pythag(double _a, double _b){
	return(sqrt(_a * _a + _b * _b));
}
double pythag(double _a, double _b, double _c){
	return(sqrt(_a * _a + _b * _b + _c * _c));
}

double logBase(double val, double base){
	return(log(val) / log(base));
}

double sine(double angle){
	return(sin(angle * 2 * PI));
}
double cosine(double angle){
	return(cos(angle * 2 * PI));
}
double arctangent(double x, double y){
	return(atan2(y, x) / 2 / PI);
}
double reRange(double val){
	return((1 + val) / 2);
}

double transition(double start, double end, double pos){
	return(start + ((end - start) * pos));
}

double bellCurve(double x, double a, double b, double c){
	return(a * exp(-pow(x - b, 2) / 2 / pow(c, 2)));
}

/*curve modes:
-1	lowpass
0	bandpass
1	highpass*/
double bellFilter(double x, double a, double b, double c, int mode = 0, double peak = 0.0, double peak_spread = 1.0){
	double output = 0;

	if (((mode == -1) && (x < b)) || ((mode == 1) && (x > b))){
		output += a;
	} else {
		output += bellCurve(x, a, b, c);
	}

	if (peak != 0.0){
		output += bellCurve(x, a * peak, b, c * peak_spread);
	}

	return(output);
}

struct complex {
	long double r;
	long double i;
	
	complex(){
		r = 0;
		i = 0;
	}

	complex(long double tr, long double ti = 0){
		r = tr;
		i = ti;
	}

	long double abs(){
		return(sqrt((r * r) + (i * i)));
	}

	//phase, in radians
	long double phase(){
		return(atan2(this->i, this->r));
	}

	complex conj(){
		return(complex(this->r, -(this->i)));
	}

	friend std::ostream& operator<<(std::ostream &stream, const complex& c){
		if (c.r == 0){
			stream << c.i;
		} else if (c.r != 0){
			stream << c.r;
			if (c.i == -1){
				stream << " - i";
			} else if (c.i < 0){
				stream << " - " << std::abs(c.i) << "i";
			} else if (c.i == 1){
				stream << " + i";
			} else if (c.i > 0){
				stream << " + " << c.i << "i";
			}
		}
		return(stream);
	}

	complex& operator=(double temp){
		r = temp;
		i = 0;
		return(*this);
	}

	complex operator+(complex temp){
		return(complex(r + temp.r, i + temp.i));
	}

	complex operator+(long double temp){
		return(complex(r + temp, i));
	}

	complex& operator+=(complex temp){
		r += temp.r;
		i += temp.i;
		return(*this);
	}

	complex& operator+=(long double temp){
		r += temp;
		return(*this);
	}

	complex operator-(complex temp){
		return(complex(r - temp.r, i - temp.i));
	}

	complex operator-(long double temp){
		return(complex(r - temp, i));
	}

	complex& operator-=(complex temp){
		r -= temp.r;
		i -= temp.i;
		return(*this);
	}

	complex& operator-=(long double temp){
		r -= temp;
		return(*this);
	}
	
	complex operator*(complex temp){
		return(complex(((r * temp.r) - (i * temp.i)), ((r * temp.i) + (i * temp.r))));
	}
	
	complex operator*(long double temp){
		return(complex(r * temp, i * temp));
	}

	complex& operator*=(complex temp){
		r = (r * temp.r) - (i * temp.i);
		i = (r * temp.i) + (i * temp.r);
		return(*this);
	}

	complex& operator*=(long double temp){
		r *= temp;
		i *= temp;
		return(*this);
	}
	
	complex operator/(complex temp){
		return(complex(((r * temp.r) + (i * temp.i)) / ((temp.r * temp.r) + (temp.i * temp.i)), ((i * temp.r) - (r * temp.i)) / ((temp.r * temp.r) + (temp.i * temp.i))));
	}
	
	complex operator/(long double temp){
		return(complex(r / temp, i / temp));
	}

	complex& operator/=(complex temp){
		r = ((r * temp.r) + (i * temp.i)) / ((temp.r * temp.r) + (temp.i * temp.i));
		i = ((i * temp.r) - (r * temp.i)) / ((temp.r * temp.r) + (temp.i * temp.i));
		return(*this);
	}

	complex& operator/=(long double temp){
		r /= temp;
		i /= temp;
		return(*this);
	}
};
complex complexPolar(long double rad, long double angle){
	return(complex(cos(angle) * rad, sin(angle) * rad));
}

complex sin(complex angle){
	return(complex(sin(angle.r) * cosh(angle.i), cos(angle.r) * sinh(angle.i)));
}
complex cos(complex angle){
	return(complex(cos(angle.r) * cosh(angle.i), -sin(angle.r) * sinh(angle.i)));
}
complex tan(complex angle){
	return(sin(angle) / cos(angle));
}
double abs(complex C){
	return(C.abs());
}

struct Pos {
	int x, y;

	Pos(){
		x = 0;
		y = 0;
	}

	Pos(int _x, int _y){
		x = _x;
		y = _y;
	}

	bool operator==(Pos _pos){
		return(x == _pos.x && y == _pos.y);
	}
	
	double abs(){
		return(pythag(x, y));
	}

	double manhattanDist(){
		return(::abs(x) + ::abs(y));
	}

	double angle(){
		return(atan2(y, x));
	}

	Pos operator-(){
		return(Pos(-x, -y));
	}

	Pos operator+(Pos _pos){
		return(Pos(x + _pos.x, y + _pos.y));
	}
	Pos operator+=(Pos _pos){
		return(*this = Pos(x + _pos.x, y + _pos.y));
	}
	Pos operator-(Pos _pos){
		return(Pos(x - _pos.x, y - _pos.y));
	}
	Pos operator-=(Pos _pos){
		return(*this = Pos(x - _pos.x, y - _pos.y));
	}
	Pos operator*(double _val){
		return(Pos(x * _val, y * _val));
	}
	Pos operator*=(double _val){
		return(*this = Pos(x * _val, y * _val));
	}
	Pos operator/(double _val){
		return(Pos(x / _val, y / _val));
	}
	Pos operator/=(double _val){
		return(*this = Pos(x / _val, y / _val));
	}
};

Pos transition(Pos start, Pos end, double pos){
	return(start + (end - start) * pos);
}

struct Posd {
	double x, y;

	Posd(double _x = 0, double _y = 0){
		x = _x;
		y = _y;
	}

	Posd(Pos pos){
		x = pos.x;
		y = pos.y;
	}

	operator Pos(){
		return(Pos(round(x), round(y)));
	}

	Posd operator-(){
		return(Posd(-x, -y));
	}

	double abs(){
		return(pythag(x, y));
	}

	double angle(){
		return(atan2(y, x));
	}

	Posd flip(){
		return(Posd(y, -x));
	}

	Posd operator+(Posd _pos){
		return(Posd(x + _pos.x, y + _pos.y));
	}
	Posd operator+=(Posd _pos){
		*this = Posd(x + _pos.x, y + _pos.y);
		return(*this);
	}
	Posd operator-(Posd _pos){
		return(Posd(x - _pos.x, y - _pos.y));
	}
	Posd operator-=(Posd _pos){
		*this = Posd(x - _pos.x, y - _pos.y);
		return(*this);
	}
	Posd operator*(double _val){
		return(Posd(x * _val, y * _val));
	}
	Posd operator*=(double _val){
		*this = Posd(x * _val, y * _val);
		return(*this);
	}
	Posd operator/(double _val){
		return(Posd(x / _val, y / _val));
	}
	Posd operator/=(double _val){
		*this = Posd(x * _val, y / _val);
		return(*this);
	}
};
struct Pos3d {
	double x = 0, y = 0, z = 0;
	Pos3d(double _x = 0, double _y = 0, double _z = 0){
		x = _x;
		y = _y;
		z = _z;
	}
	Pos3d operator+(Pos3d _pos){
		return(Pos3d(x + _pos.x, y + _pos.y, z + _pos.z));
	}
	Pos3d operator+=(Pos3d _pos){
		*this = Pos3d(x + _pos.x, y + _pos.y, z + _pos.z);
		return(*this);
	}
	Pos3d operator-(Pos3d _pos){
		return(Pos3d(x - _pos.x, y - _pos.y, z - _pos.z));
	}
	Pos3d operator-=(Pos3d _pos){
		*this = Pos3d(x - _pos.x, y - _pos.y, z - _pos.z);
		return(*this);
	}
	Pos3d operator*(double val){
		return(Pos3d(x * val, y * val, z * val));
	}
	Pos3d operator*=(double val){
		*this = Pos3d(x * val, y * val, z * val);
		return(*this);
	}
	Pos3d operator/(double val){
		return(Pos3d(x / val, y / val, z / val));
	}
	Pos3d operator/=(double val){
		*this = Pos3d(x / val, y / val, z / val);
		return(*this);
	}
	double abs(){
		return(sqrt((x * x) + (y * y) + (z * z)));
	}
};
Posd Posd_Polar(double ang, double rad = 1){
	return(Posd(rad * cos(ang), rad * sin(ang)));
}
struct Rect {
	int x, y, w, h;
	
	Rect(){
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}
	Rect(int _w, int _h){
		x = 0;
		y = 0;
		w = _w;
		h = _h;
	}
	Rect(int _x, int _y, int _w, int _h){
		x = _x;
		y = _y;
		w = _w;
		h = _h;
	}
	Rect(Pos pos, Pos size){
		x = pos.x;
		y = pos.y;
		w = size.x;
		h = size.y;
	}
	Rect(Pos size){
		x = 0;
		y = 0;
		w = size.x;
		h = size.y;
	}

	bool hit(Pos _point){
		return(_point.x >= x && _point.x < x + w && _point.y >= y && _point.y < y + h);
	}

	Pos pos(){
		return(Pos(x, y));
	}

	Pos size(){
		return(Pos(w, h));
	}

	Pos center(){
		return(Pos(x + w / 2, y + h / 2));
	}
	
	Rect operator+(Pos pos){
		return(Rect(x + pos.x, y + pos.y, w, h));
	}
	Rect operator+=(Pos pos){
		return(*this = Rect(x + pos.x, y + pos.y, w, h));
	}

	Rect restrict(Rect bounds, bool resize = false){
		if (resize && w > bounds.w){
			w = bounds.w;
		}
		if (x + w > bounds.x + bounds.w){
			x = bounds.x + bounds.w - w;
		}
		if (x < bounds.x){
			x = bounds.x;
		}
		if (resize && h > bounds.h){
			h = bounds.w;
		}
		if (y + h > bounds.y + bounds.h){
			y = bounds.y + bounds.h - h;
		}
		if (y < bounds.y){
			y = bounds.y;
		}
		return(*this);
	}

	Rect restrict(Pos bounds, bool resize = false){
		if (resize && w > bounds.x){
			w = bounds.x;
		}
		if (x + w > bounds.x){
			x = bounds.x - w;
		}
		if (x < 0){
			x = 0;
		}
		if (resize && h > bounds.y){
			h = bounds.y;
		}
		if (y + h > bounds.y){
			y = bounds.y - h;
		}
		if (y < 0){
			y = 0;
		}
		return(*this);
	}

	bool intersects(const Rect& trect) const {
		return((w > 0) && (h > 0) && (trect.w > 0) && (trect.h > 0) && (trect.x < x + w) && (trect.x + trect.w >= x) && (trect.y < y + h) && (trect.y + trect.h >= y));
	}

#ifdef _SDL_H
	operator SDL_Rect(){
		SDL_Rect trect;
		trect.x = x;
		trect.y = y;
		trect.w = w;
		trect.h = h;
		return(trect);
	}
#endif
};

void FFT(complex *x, int n){

	double *_x = (double*)x;

	// reverse-binary reindexing
	int _n = n << 1;
	int j = 1, m;
	double temp;
	for (int i = 1; i < _n ; i += 2){
		if (j > i){
			temp = _x[i - 1];
			_x[i - 1] = _x[j - 1];
			_x[j - 1] = temp;

			temp = _x[i];
			_x[i] = _x[j];
			_x[j] = temp;
		}
		m = n;
		while (m >= 2 && j > m){
			j -= m;
			m >>= 1;
		}
		j += m;
	};
 
	// here begins the Danielson-Lanczos section
	double wtemp, wr, wpr, wpi, wi, theta;
	double tempr, tempi;
	int istep, mmax = 2;
	while (_n > mmax){
		istep = mmax << 1;
		theta = -2 * PI / mmax;
		wtemp = sin(0.5 * theta);
		wpr = -2.0 * wtemp * wtemp;
		wpi = sin(theta);
		wr = 1.0;
		wi = 0.0;
		for (int m = 1; m < mmax; m += 2){
			for (int i = m; i <= _n; i += istep){
				j = i + mmax;
				tempr = wr * _x[j - 1] - wi * _x[j];
				tempi = wr * _x[j] + wi * _x[j - 1];
 
				_x[j - 1] = _x[i - 1] - tempr;
				_x[j] = _x[i] - tempi;
				_x[i - 1] += tempr;
				_x[i] += tempi;
			}
			wtemp = wr;
			wr += wr * wpr - wi * wpi;
			wi += wi * wpr + wtemp * wpi;
		}
		mmax = istep;
	}
}

void iFFT(complex *x, int n){

	for (int i = 0; i < n; i++){
		x[i].i = -x[i].i;
	}

	FFT(x, n);

	double norm = 1 / double(n);
	for (int i = 0; i < n; i++){
		x[i] = x[i].conj() * norm;
	}
}

std::string toLowercase(const std::string& str){
	std::string res;
	res.resize(str.size());
	for (int i = 0; i < str.size(); i++){
		res[i] = std::tolower(str[i]);
	}
	return res;
}