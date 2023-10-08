#pragma once

#include <mymaths.h>

struct Color {
	public:

	void fromInt(Uint32 _color){
		r = ((_color & 0xFF0000) / 0x10000) / 255.0;
		g = ((_color & 0xFF00) / 0x100) / 255.0;
		b = (_color & 0xFF) / 255.0;
		a = ((_color & 0xFF000000) / 0x1000000) / 255.0;
		updateFromRGB();
	}

	operator Uint32(){
		return(toInt());
	}
	
	Color (Uint32 _color = 0xFF000000){
		fromInt(_color);
	}
	
	Color (double _r, double _g, double _b, double _a = 1){
		setR(r);
		setG(g);
		setB(b);
		setA(a);
	}
	
	Color operator=(Uint32 _color){
		fromInt(_color);
		return(*this);
	}

	void setColor(Color _color){
		*this = _color;
	}

	void setR(double val){
		r = std::min(std::max(val, 0.0), 1.0);
		updateFromRGB();
	}
	void setRi(Uint8 val){
		r = val / 255.0;
		updateFromRGB();
	}
	double getR(){
		return(r);
	}
	Uint8 getRi(){
		return((Uint8)(r * 255));
	}

	void setG(double val){
		g = std::min(std::max(val, 0.0), 1.0);
		updateFromRGB();
	}
	void setGi(Uint8 val){
		g = val / 255.0;
		updateFromRGB();
	}
	double getG(){
		return(g);
	}
	Uint8 getGi(){
		return((Uint8)(g * 255));
	}

	void setB(double val){
		b = std::min(std::max(val, 0.0), 1.0);
		updateFromRGB();
	}
	void setBi(Uint8 val){
		b = val / 255.0;
		updateFromRGB();
	}
	double getB(){
		return(b);
	}
	Uint8 getBi(){
		return((Uint8)(b * 255));
	}
	
	void setH(double val){
		h = val - floor(val);
		updateFromHSV();
	}
	double getH(){
		return(h);
	}

	void setS(double val){
		s = std::min(std::max(val, 0.0), 1.0);
		updateFromHSV();
	}
	double getS(){
		return(s);
	}

	void setV(double val){
		v = std::min(std::max(val, 0.0), 1.0);
		updateFromHSV();
	}
	double getV(){
		return(v);
	}
	
	void setA(double val){
		a = std::min(std::max(val, 0.0), 1.0);
	}
	void setAi(Uint8 val){
		a = val / 255.0;
	}
	double getA(){
		return(a);
	}
	Uint8 getAi(){
		return((Uint8)(a * 255));
	}

	Uint32 toInt(){
		return(((int)(a * 255) * 0x1000000) + ((int)(r * 255) * 0x10000) + ((int)(g * 255) * 0x100) + (int)(b * 255));
	}

	Color operator+(Color);

	Color operator+=(Color);

	private:
	double r, g, b, h, s, v, a;

	Uint8 rainbowR(double pos){
		pos = pos - floor(pos);
		if (pos < 1 / 6.0 || pos > 5 / 6.0){
			 return(255);
		} else if (pos < 2 / 6.0){
			return((int)(255 * (1 - ((pos - 1 / 6.0) * 6))));
		} else if (pos > 4 / 6.0){
			return((int)(255 * ((pos - 4 / 6.0) * 6)));
		} else {
			return(0);
		}
	}

	Uint8 rainbowG(double pos){
		pos = pos - floor(pos);
		if (pos < 1 / 6.0){
			return((int)(255 * (pos * 6)));
		} else if (pos < 3 / 6.0){
			return(255);
		} else if (pos < 4 / 6.0){
			return((int)(255 * (1 - ((pos - 3 / 6.0) * 6))));
		} else {
			return(0);
		}
	}

	Uint8 rainbowB(double pos){
		pos = pos - floor(pos);
		if (pos < 2 / 6.0){
			return(0);
		} else if (pos < 3 / 6.0){
			return((int)(255 * ((pos - 2 / 6.0) * 6)));
		} else if (pos < 5 / 6.0){
			return(255);
		} else {
			return((int)(255 * (1 - ((pos - 5 / 6.0) * 6))));
		}
	}

	void updateFromRGB(){
		v = (std::min(r, std::min(g, b)) + std::max(r, std::max(g, b))) / 2;

		s = (std::max(r, std::max(g, b)) - std::min(r, std::min(g, b)));

		if (s == 0){
			h = 0;
		} else if (std::max(r, std::max(g, b)) == r){
			h = (((g - b) / s / 6) - floor((g - b) / s / 6)) * 6;
		} else if (std::max(r, std::max(g, b)) == g){
			h = (b - r) / s + 2;
		} else if (std::max(r, std::max(g, b)) == b){
			h = (r - g) / s + 4;
		}
		h /= 6;
	}

	void updateFromHSV(){
		if (v <= 0.5){
			r = transition(0, transition(0.5, rainbowR(h) / 255.0, s), v * 2);
			g = transition(0, transition(0.5, rainbowG(h) / 255.0, s), v * 2);
			b = transition(0, transition(0.5, rainbowB(h) / 255.0, s), v * 2);
		} else {
			r = transition(1, transition(0.5, rainbowR(h) / 255.0, s), (1 - v) * 2);
			g = transition(1, transition(0.5, rainbowG(h) / 255.0, s), (1 - v) * 2);
			b = transition(1, transition(0.5, rainbowB(h) / 255.0, s), (1 - v) * 2);
		}
	}
};

Color ColorHSVA(double _h, double _s, double _v, double _a = 1){
	Color temp = Color(0);
	temp.setH(_h);
	temp.setS(_s);
	temp.setV(_v);
	temp.setA(_a);
	return(temp);
}

Color ColorRGBA(double _r, double _g, double _b, double _a = 1){
	Color temp = Color(0);
	temp.setR(_r);
	temp.setG(_g);
	temp.setB(_b);
	temp.setA(_a);
	return(temp);
}

Color Color::operator+(Color tcolor){
	return(ColorRGBA(
		(this->getR() * (1 - tcolor.getA())) + (tcolor.getR() * tcolor.getA()),
		(this->getG() * (1 - tcolor.getA())) + (tcolor.getG() * tcolor.getA()),
		(this->getB() * (1 - tcolor.getA())) + (tcolor.getB() * tcolor.getA()),
		tcolor.getA() + (this->getA() * (1 - tcolor.getA()))));
}

Color Color::operator +=(Color tcolor){
	return(*this = ColorRGBA(
		this->getR() * (1 - tcolor.getA()) + tcolor.getR() * tcolor.getA(),
		this->getG() * (1 - tcolor.getA()) + tcolor.getG() * tcolor.getA(),
		this->getB() * (1 - tcolor.getA()) + tcolor.getB() * tcolor.getA(),
		tcolor.getA() + this->getA() * (1 - tcolor.getA())));
}

double getColorDiffRGB(Color color1, Color color2){
	return((abs(color1.getR() - color2.getR()) + abs(color1.getG() - color2.getG()) + abs(color1.getB() - color2.getB())) / 3.0);
}

double getColorDiffRGBA(Color color1, Color color2){
	return((abs(color1.getR() - color2.getR()) + abs(color1.getG() - color2.getG()) + abs(color1.getB() - color2.getB()) + abs(color1.getA() - color2.getA())) / 4.0);
}

Color transition(Color color1, Color color2, double pos){
	return(ColorRGBA(transition(color1.getR(), color2.getR(), pos), transition(color1.getG(), color2.getG(), pos), transition(color1.getB(), color2.getB(), pos), transition(color1.getA(), color2.getA(), pos)));
}