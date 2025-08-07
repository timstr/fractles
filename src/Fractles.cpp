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

int main(int argc, char** argv){
    GUI::init(Pos(1000, 700), "Fractles");

    std::optional<std::string> json_path = std::nullopt;
    if (argc == 2) {
        json_path = std::string(argv[1]);
    }

    g_fractal().init(1000, 700, json_path);

    FractalWindow* fractal_window = new FractalWindow();

    GUI::rootwindow.addElement(fractal_window);

    fractal_window->grabFocus();

    GUI::run();

    return 0;
}
