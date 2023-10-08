#include <gui.h>
#include <thread>
#include <gradient.h>
#include <image.h>
#include <SDL.h>
#include <sstream>

#include "rendering.h"

#include "uicomponents.h"

#include "gradienteditor.h"

////////////////////////////////////

////////////////////////////////////

#include "fractalwindow.h"

int main(int argc, char** args){

	GUI::init(Pos(1000, 700), "Fractles");

	g_fractal().init(1000, 700);

	FractalWindow* fractal_window = new FractalWindow();

	GUI::rootwindow.addElement(fractal_window);

	fractal_window->grabFocus();

	GUI::run();

	return 0;
}