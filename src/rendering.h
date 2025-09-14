#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <optional>
#include <thread>

#include <json.hpp>

const size_t worker_count = std::thread::hardware_concurrency();

struct Fractal;

Fractal& g_fractal();

struct Fractal {
    struct Parameters {
        double x_offset = 0.0;
        double y_offset = 0.0;
        double magnification = 0.5;
        double rotation = 0;
        double iteration_limit = 1000.0;
        double escape_limit = 1000.0;
        bool Z_init = false;
        bool julia = false;
        double julia_r = 0.1;
        double julia_i = 0;
        bool J_multiply = false;
        double J_multiplier_r = 1.0;
        double J_multiplier_i = 1.0;
        bool square_Z = true;
        bool pre_add = false;
        bool mid_add = true;
        bool post_add = false;
        bool box_reflect = false;
        double box_reflect_scale = 1.0;
        bool ring_reflect = false;
        double ring_reflect_scale = 1.0;
        bool sponge = false;
        double sponginess = 0;
        bool Z_multiply = false;
        double Z_multiplier_r = 1.0;
        double Z_multiplier_i = 0.0;
        bool C_multiply = false;
        double C_multiplier_r = 1.0;
        double C_multiplier_i = 0.0;
        Gradient gradient;
        bool shade = false;
        double highlight_strength = 0.2;
        double shade_strength = 0.2;
        std::string texture_path;
        std::optional<Image> texture;
        double texture_x_offset = 0.0;
        double texture_x_scale = 1.0;
        double texture_y_offset = 0.0;
        double texture_y_scale = 1.0;

        nlohmann::json to_json() const {
            auto j = nlohmann::json::object();
            j["x_offset"] = this->x_offset;
            j["y_offset"] = this->y_offset;
            j["magnification"] = this->magnification;
            j["rotation"] = this->rotation;
            j["iteration_limit"] = this->iteration_limit;
            j["escape_limit"] = this->escape_limit;
            j["Z_init"] = this->Z_init;
            j["julia"] = this->julia;
            j["julia_r"] = this->julia_r;
            j["julia_i"] = this->julia_i;
            j["J_multiply"] = this->J_multiply;
            j["J_multiplier_r"] = this->J_multiplier_r;
            j["J_multiplier_i"] = this->J_multiplier_i;
            j["square_Z"] = this->square_Z;
            j["pre_add"] = this->pre_add;
            j["mid_add"] = this->mid_add;
            j["post_add"] = this->post_add;
            j["box_reflect"] = this->box_reflect;
            j["box_reflect_scale"] = this->box_reflect_scale;
            j["ring_reflect"] = this->ring_reflect;
            j["ring_reflect_scale"] = this->ring_reflect_scale;
            j["sponge"] = this->sponge;
            j["sponginess"] = this->sponginess;
            j["Z_multiply"] = this->Z_multiply;
            j["Z_multiplier_r"] = this->Z_multiplier_r;
            j["Z_multiplier_i"] = this->Z_multiplier_i;
            j["C_multiply"] = this->C_multiply;
            j["C_multiplier_r"] = this->C_multiplier_r;
            j["C_multiplier_i"] = this->C_multiplier_i;

            j["gradient"] = this->gradient.to_json();

            j["shade"] = this->shade;
            j["highlight_strength"] = this->highlight_strength;
            j["shade_strength"] = this->shade_strength;

            if (this->texture.has_value()) {
                j["texture_path"] = this->texture_path;
                j["texture_x_offset"] = this->texture_x_offset;
                j["texture_x_scale"] = this->texture_x_scale;
                j["texture_y_offset"] = this->texture_y_offset;
                j["texture_y_scale"] = this->texture_y_scale;
            }

            return j;
        }

        void from_json(const nlohmann::json& j) {
            this->x_offset = j["x_offset"].get<double>();
            this->y_offset = j["y_offset"].get<double>();
            this->magnification = j["magnification"].get<double>();
            this->rotation = j["rotation"].get<double>();
            this->iteration_limit = j["iteration_limit"].get<double>();
            this->escape_limit = j["escape_limit"].get<double>();
            this->Z_init = j["Z_init"].get<bool>();
            this->julia = j["julia"].get<bool>();
            this->julia_r = j["julia_r"].get<double>();
            this->julia_i = j["julia_i"].get<double>();
            this->J_multiply = j["J_multiply"].get<bool>();
            this->J_multiplier_r = j["J_multiplier_r"].get<double>();
            this->J_multiplier_i = j["J_multiplier_i"].get<double>();
            this->square_Z = j["square_Z"].get<bool>();
            this->pre_add = j["pre_add"].get<bool>();
            this->mid_add = j["mid_add"].get<bool>();
            this->post_add = j["post_add"].get<bool>();
            this->box_reflect = j["box_reflect"].get<bool>();
            this->box_reflect_scale = j["box_reflect_scale"].get<double>();
            this->ring_reflect = j["ring_reflect"].get<bool>();
            this->ring_reflect_scale = j["ring_reflect_scale"].get<double>();
            this->sponge = j["sponge"].get<bool>();
            this->sponginess = j["sponginess"].get<double>();
            this->Z_multiply = j["Z_multiply"].get<bool>();
            this->Z_multiplier_r = j["Z_multiplier_r"].get<double>();
            this->Z_multiplier_i = j["Z_multiplier_i"].get<double>();
            this->C_multiply = j["C_multiply"].get<bool>();
            this->C_multiplier_r = j["C_multiplier_r"].get<double>();
            this->C_multiplier_i = j["C_multiplier_i"].get<double>();

            this->gradient.from_json(j["gradient"]);

            this->shade = j.value("shade", false);
            this->highlight_strength = j.value("highlight_strength", 0.0);
            this->shade_strength = j.value("shade_strength", 0.0);

            std::string path = j.value("texture_path", std::string(""));

            if (!path.empty()) {
                this->texture_path = path;
                this->texture.emplace(path);
                this->texture_x_offset = j["texture_x_offset"].get<double>();
                this->texture_x_scale = j["texture_x_scale"].get<double>();
                this->texture_y_offset = j["texture_y_offset"].get<double>();
                this->texture_y_scale = j["texture_y_scale"].get<double>();
            }
        }
    } params;
    bool rerender = true;

    void init(size_t _width, size_t _height, std::optional<std::string> json_path) {
        workers.resize(worker_count);
        for (size_t i = 0; i < worker_count; ++i) {
            workers[i] = std::make_unique<Worker>();
        }

        width = _width;
        height = _height;

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        if (json_path.has_value()) {
            auto f = std::ifstream(*json_path);
            params.from_json(nlohmann::json::parse(f));
        }
    }

    void randomize_gradient() {
        this->params.gradient.randomize(5 + rand() % 11);
    }

    void load_texture(std::string path) {
        this->params.texture_path = path;
        this->params.texture.emplace(path);

        // Avoid a race condition due to locking
        // the texture from multiple threads
        this->params.texture->getPixel(0, 0);
    }

    ~Fractal(){
        abortRender();
        SDL_DestroyTexture(texture);
    }

    size_t w(){
        return width;
    }

    size_t h(){
        return height;
    }

    void resize(size_t _width, size_t _height){
        abortRender();
        width = _width;
        height = _height;

        if (texture){
            SDL_DestroyTexture(texture);
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }

    void beginRender(){
        abortRender();
        lockTexture();
        for (size_t i = 0; i < workers.size(); i++){
            workers[i]->begin(i, renders);
        }
        renders++;
    }

    void beginAntiAlias(){
        abortRender();
        lockTexture();
        for (size_t i = 0; i < workers.size(); i++){
            workers[i]->beginAntiAlias(i, renders);
        }
        renders++;
    }

    void abortRender(){
        for (size_t i = 0; i < workers.size(); i++){
            workers[i]->abort();
        }
        unlockTexture();
    }

    bool isComplete(){
        for (size_t i = 0; i < workers.size(); i++){
            if (!workers[i]->isComplete()){
                return false;
            }
        }
        return true;
    }

    bool isAntiAliased(){
        for (size_t i = 0; i < workers.size(); i++){
            if (!workers[i]->isAntiAliased()){
                return false;
            }
        }
        return true;
    }

    double getProgress(){
        if (isComplete()){
            return 1.0;
        }
        size_t sum = 0;
        for (size_t i = 0; i < worker_count; i++){
            sum += workers[i]->getPixelsRendered();
        }
        return sum / (double)(g_fractal().w() * g_fractal().h());
    }

    void save(){
        if (!isComplete() || !isAntiAliased()){
            return;
        }

        if (!texture){
            throw std::runtime_error("No texture?");
        }

        SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);

        for (size_t x = 0; x < width; x++){
            for (size_t y = 0; y < height; y++){
                ((Uint32*)surface->pixels)[(y * surface->pitch / sizeof(Uint32)) + x] = pixel_data[(y * pitch) + x];
            }
        }

        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::stringstream ss;
        ss << "Fractal " << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
        const std::string path_without_ext = ss.str();

        IMG_SavePNG(surface, (path_without_ext + ".png").c_str());

        SDL_FreeSurface(surface);

        auto o = std::ofstream(path_without_ext + ".json");
        o << this->params.to_json() << std::endl;

        std::cout << "Saved " << width << 'x' << height << " image to \"" << path_without_ext << "\"" << std::endl;

    }

    void render(){
        if (!isComplete()){
            renderLock();
            unlockTexture();
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            lockTexture();
            renderUnlock();
        } else {
            unlockTexture();
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            lockTexture();
        }
    }

    void renderLock(){
        for (size_t i = 0; i < workers.size(); i++){
            workers[i]->pause();
        }
        for (size_t i = 0; i < workers.size(); i++){
            while (!workers[i]->isWaiting()){
                // TODO: fix this busy wait?
            }
        }
    }

    void renderUnlock(){
        for (size_t i = 0; i < workers.size(); i++){
            workers[i]->resume();
        }
    }

    void lockTexture(){
        if (!locked){
            int p;
            SDL_LockTexture(texture, NULL, (void**)&pixel_data, &p);
            p /= sizeof(Uint32);
            this->pitch = static_cast<size_t>(p);
            this->locked = true;
        }
    }

    void unlockTexture(){
        if (locked){
            SDL_UnlockTexture(texture);
            this->pixel_data = nullptr;
            this->locked = false;
        }
    }

    inline void setPixel(size_t x, size_t y, Uint32 color){
        pixel_data[(y * pitch) + x] = color;
    }

    inline bool pixelContrast(size_t x, size_t y){
        Uint32 color;
        int r, g, b, ar, ag, ab;

        color = pixel_data[(y * pitch) + x];
        r = color & 0xFF0000 / 0x10000;
        g = color & 0xFF00 / 0x100;
        b = color & 0xFF;
        const int threshold = 16;
        if (x > 0){
            color = pixel_data[(y * pitch) + x - 1];
            ar = color & 0xFF0000 / 0x10000;
            ag = color & 0xFF00 / 0x100;
            ab = color & 0xFF;
            if (std::abs(r - ar) + std::abs(g - ag) + std::abs(b - ab) > threshold){
                return true;
            }
        }
        if (x < width - 1){
            color = pixel_data[(y * pitch) + x + 1];
            ar = color & 0xFF0000 / 0x10000;
            ag = color & 0xFF00 / 0x100;
            ab = color & 0xFF;
            if (std::abs(r - ar) + std::abs(g - ag) + std::abs(b - ab) > threshold){
                return true;
            }
        }
        if (y > 0){
            color = pixel_data[((y - 1) * pitch) + x];
            ar = color & 0xFF0000 / 0x10000;
            ag = color & 0xFF00 / 0x100;
            ab = color & 0xFF;
            if (std::abs(r - ar) + std::abs(g - ag) + std::abs(b - ab) > threshold){
                return true;
            }
        }
        if (y < height - 1){
            color = pixel_data[((y + 1) * pitch) + x];
            ar = color & 0xFF0000 / 0x10000;
            ag = color & 0xFF00 / 0x100;
            ab = color & 0xFF;
            if (std::abs(r - ar) + std::abs(g - ag) + std::abs(b - ab) > threshold){
                return true;
            }
        }
        return false;
    }

    private:
    size_t width, height;

    SDL_Texture* texture = nullptr;
    Uint32* pixel_data = nullptr;
    bool locked = false;
    size_t pitch;
    size_t renders = 0;

    struct Worker {
        Worker() {

        }

        void begin(int offset, int carryover){
            abort();
            data.offset = offset;
            data.carryover = carryover;
            data.complete = false;
            data.abort = false;
            data.antialias = false;
            data.antialias_complete = false;
            data.pixels_rendered = 0;
            thread = std::thread(render, std::ref(data));
        }

        void beginAntiAlias(int offset, int carryover){
            abort();
            data.offset = offset;
            data.carryover = carryover;
            data.complete = false;
            data.abort = false;
            data.antialias = true;
            data.antialias_complete = false;
            data.pixels_rendered = 0;
            thread = std::thread(render, std::ref(data));
        }

        void abort(){
            data.abort = true;
            if (thread.joinable()){
                thread.join();
            }
        }

        void pause(){
            data.pause = true;
        }

        void resume(){
            data.pause = false;
        }

        bool isWaiting(){
            return data.complete || data.waiting;
        }

        bool isComplete(){
            return data.complete;
        }

        bool isAntiAliased(){
            return data.antialias_complete;
        }

        Uint32 getPixelsRendered(){
            return data.pixels_rendered;
        }

        private:
        struct Data {
            int offset = 0;
            int carryover = 0;
            std::atomic<bool> complete = true;
            std::atomic<bool> abort = false;
            std::atomic<bool> pause = false;
            std::atomic<bool> waiting = false;
            std::atomic<bool> antialias = false;
            std::atomic<bool> antialias_complete = false;
            std::atomic<Uint32> pixels_rendered = 0;
        };
        Data data;
        std::thread thread;

        static void render(Data& data){
            if (data.antialias){
                unsigned int size = g_fractal().w() * g_fractal().h();
                for (size_t i = data.offset; i < size && !data.abort; i += worker_count){

                    int _i = (i * (long long)2256913) % size;
                    int _y = _i / g_fractal().w();
                    int _x = (_i + data.carryover) % g_fractal().w();

                    if (g_fractal().pixelContrast(_x, _y)){
                        Uint32 tcolor;
                        unsigned int a = 0, r = 0, g = 0, b = 0;
                        for (double ax = -0.5; ax < 0.5; ax += 0.25){
                            for (double ay = -0.5; ay < 0.5; ay += 0.25){
                                tcolor = renderPixel(_x + ax, _y + ay);
                                a += (tcolor & 0xFF000000) >> 24;
                                r += (tcolor & 0xFF0000) >> 16;
                                g += (tcolor & 0xFF00) >> 8;
                                b += tcolor & 0xFF;
                            }
                        }
                        a /= 16;
                        r /= 16;
                        g /= 16;
                        b /= 16;
                        tcolor = a * 0x1000000 + r * 0x10000 + g * 0x100 + b;
                        g_fractal().setPixel(_x, _y, tcolor);
                    }

                    data.pixels_rendered += 1;

                    if (data.pause){
                        data.waiting = true;
                        while (data.pause && !data.abort){

                        }
                        data.waiting = false;
                    }
                }
                data.antialias_complete = true;
            } else {
                unsigned int size = g_fractal().w() * g_fractal().h();
                for (size_t i = data.offset; i < size && !data.abort; i += worker_count){

                    int _i = ((i + data.carryover) * (long long)2256913) % size;
                    int _y = _i / g_fractal().w();
                    int _x = (_i + data.carryover) % g_fractal().w();

                    g_fractal().setPixel(_x, _y, renderPixel(_x, _y));

                    data.pixels_rendered += 1;

                    if (data.pause){
                        data.waiting = true;
                        while (data.pause && !data.abort){

                        }
                        data.waiting = false;
                    }
                }
            }
            data.complete = true;
        }

        static inline double evaluateFractal(double x, double y) {
            Fractal& fractal = g_fractal();
            Parameters& params = fractal.params;

            double angle = atan2(y - fractal.h() / 2, x - fractal.w() / 2) + params.rotation;
            double dist = pythag(x - fractal.w() / 2, y - fractal.h() / 2);

            x = dist * cos(angle) + fractal.w() / 2;
            y = dist * sin(angle) + fractal.h() / 2;

            double _x = (double)(x - fractal.w() / 2) / std::min(fractal.w(), fractal.h()) / params.magnification + params.x_offset;
            double _y = (double)(y - fractal.h() / 2) / std::min((double)fractal.w(), (double)fractal.h()) / params.magnification + params.y_offset;

            complex Z = 0.0;
            complex C = complex(_y, _x);
            complex J = complex(params.julia_r, params.julia_i);
            complex Zx = complex(params.Z_multiplier_r, params.Z_multiplier_i);
            complex Cx = complex(params.C_multiplier_r, params.C_multiplier_i);
            complex Jx = complex(params.J_multiplier_r, params.J_multiplier_i);

            if (params.Z_init){
                Z = C;
            }

            double it = 0;

            while (it < params.iteration_limit && Z.r * Z.r + Z.i * Z.i < params.escape_limit * params.escape_limit){

                if (params.pre_add){
                    Z += C;
                }

                if (params.square_Z){
                    Z = Z * Z;
                }

                if (params.julia){
                    Z += J;
                }

                if (params.J_multiply){
                    J *= Jx;
                }

                if (params.mid_add){
                    Z += C;
                }

                if (params.sponge){
                    Z += complex(params.sponginess) / Z;
                }

                if (params.box_reflect){
                    if (Z.r > params.box_reflect_scale){
                        Z.r = params.box_reflect_scale * 2 - Z.r;
                    } else if (Z.r < -params.box_reflect_scale){
                        Z.r = params.box_reflect_scale * -2 - Z.r;
                    }

                    if (Z.i > params.box_reflect_scale){
                        Z.i = params.box_reflect_scale * 2 - Z.i;
                    } else if (Z.i < -params.box_reflect_scale){
                        Z.i = params.box_reflect_scale * -2 - Z.i;
                    }
                }

                if (params.ring_reflect){
                    double abssqr = (Z.r * Z.r + Z.i * Z.i) * params.ring_reflect_scale * params.ring_reflect_scale;
                    if (sqrt(abssqr) < 0.25){
                        Z *= 4 * params.ring_reflect_scale;
                    } else if (abssqr < 1){
                        Z /= abssqr;
                    }
                }

                if (params.post_add){
                    Z += C;
                }

                if (params.Z_multiply){
                    Z *= Zx;
                }

                if (params.C_multiply){
                    C *= Cx;
                }

                it += 1;
            }

            if (it < params.iteration_limit){
                it += (1 - (log(log(Z.abs())) / log(2)));
            }

            return it / params.iteration_limit;
        }

        static inline Uint32 renderPixel(double x, double y){
            const double v = evaluateFractal(x, y);

            Parameters& params = g_fractal().params;

            Color color;
            double angle = 0.0;

            if (params.shade || params.texture.has_value()) {
                const double delta = 0.1;
                const double dx = (evaluateFractal(x + delta, y) - v) / delta;
                const double dy = (evaluateFractal(x, y + delta) - v) / delta;
                angle = std::atan2(dy, dx);
            }

            if (params.texture.has_value()) {
                double a = 0.5 + angle / (2.0 * PI);
                double xscale = params.texture_x_scale == 0.0 ? 0.0 : std::exp(params.texture_x_scale - 1.0);
                double yscale = params.texture_y_scale == 0.0 ? 0.0 : std::exp(params.texture_y_scale - 1.0);
                const double pxd = (triangle(a * xscale + params.texture_x_offset)) * static_cast<double>(params.texture->width());
                const double pyd = (triangle(v * yscale + params.texture_y_offset)) * static_cast<double>(params.texture->height());
                const double fpxd = std::floor(pxd);
                const double fpyd = std::floor(pyd);
                const double dx = pxd - fpxd;
                const double dy = pyd - fpyd;
                const int pxi = static_cast<int>(fpxd);
                const int pyi = static_cast<int>(fpyd);
                Color c00 = params.texture->getPixel(pxi % params.texture->width(), pyi % params.texture->height());
                Color c10 = params.texture->getPixel((pxi + 1) % params.texture->width(), pyi % params.texture->height());
                Color c01 = params.texture->getPixel(pxi % params.texture->width(), (pyi + 1) % params.texture->height());
                Color c11 = params.texture->getPixel((pxi + 1) % params.texture->width(), (pyi + 1) % params.texture->height());
                Color c0 = Color(
                    (1.0 - dx) * c00.getR() + dx * c10.getR(),
                    (1.0 - dx) * c00.getG() + dx * c10.getG(),
                    (1.0 - dx) * c00.getB() + dx * c10.getB()
                );
                Color c1 = Color(
                    (1.0 - dx) * c01.getR() + dx * c11.getR(),
                    (1.0 - dx) * c01.getG() + dx * c11.getG(),
                    (1.0 - dx) * c01.getB() + dx * c11.getB()
                );
                color.setR((1.0 - dy) * c0.getR() + dy * c1.getR());
                color.setG((1.0 - dy) * c0.getG() + dy * c1.getG());
                color.setB((1.0 - dy) * c0.getB() + dy * c1.getB());
            } else {
                color = params.gradient.getColorAt(v);
            }

            if (params.shade) {
                const double sin_angle = std::sin(angle);
                const double color_change = sin_angle * (
                    sin_angle > 0
                    ? params.highlight_strength
                    : params.shade_strength
                );

                color.setV(color.getV() + color_change);
                color.setS(color.getS() + std::abs(color_change));
            }

            return color.toInt();
        }
    };
    std::vector<std::unique_ptr<Worker>> workers;
};

Fractal g_fractal_instance = {};

Fractal& g_fractal() {
    return g_fractal_instance;
}
