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
    std::optional<std::string> texture_path = std::nullopt;

    int i = 1;

    while (i < argc) {
        if (argv[i] == std::string("--texture")) {
            if (i + 1 == argc) {
                std::cerr << "Missing texture path.\n";
                return 1;
            }
            texture_path = argv[i + 1];
            i += 2;
        } else {
            json_path = std::string(argv[1]);
            i += 1;
        }
    }

    g_fractal().init(1000, 700, json_path);

    if (!json_path.has_value() && !texture_path.has_value()) {
        g_fractal().randomize_gradient();
    }

    if (texture_path.has_value()) {
        g_fractal().load_texture(*texture_path);
    }

    FractalWindow* fractal_window = new FractalWindow();

    GUI::rootwindow.addElement(fractal_window);

    fractal_window->grabFocus();

    GUI::run();

    return 0;
}
