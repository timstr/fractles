#pragma once

#include <atomic>
#include <memory>
#include <thread>

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
    } params;
    bool rerender = true;

    void init(size_t _width, size_t _height){
        workers.resize(worker_count);
        for (size_t i = 0; i < worker_count; ++i) {
            workers[i] = std::make_unique<Worker>();
        }

        width = _width;
        height = _height;

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        params.gradient.randomize(5 + rand() % 11);
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
        if (isComplete() && isAntiAliased()){
            if (!texture){
                throw;
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
            ss << "Fractal " << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << ".png";

            IMG_SavePNG(surface, ss.str().c_str());

            SDL_FreeSurface(surface);
        }
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

        static inline Uint32 renderPixel(double x, double y){

            double angle = atan2(y - g_fractal().h() / 2, x - g_fractal().w() / 2) + g_fractal().params.rotation;
            double dist = pythag(x - g_fractal().w() / 2, y - g_fractal().h() / 2);

            x = dist * cos(angle) + g_fractal().w() / 2;
            y = dist * sin(angle) + g_fractal().h() / 2;

            double _x = (double)(x - g_fractal().w() / 2) / std::min(g_fractal().w(), g_fractal().h()) / g_fractal().params.magnification + g_fractal().params.x_offset;
            double _y = (double)(y - g_fractal().h() / 2) / std::min((double)g_fractal().w(), (double)g_fractal().h()) / g_fractal().params.magnification + g_fractal().params.y_offset;

            complex Z = 0.0;
            complex C = complex(_y, _x);
            complex J = complex(g_fractal().params.julia_r, g_fractal().params.julia_i);
            complex Zx = complex(g_fractal().params.Z_multiplier_r, g_fractal().params.Z_multiplier_i);
            complex Cx = complex(g_fractal().params.C_multiplier_r, g_fractal().params.C_multiplier_i);
            complex Jx = complex(g_fractal().params.J_multiplier_r, g_fractal().params.J_multiplier_i);

            if (g_fractal().params.Z_init){
                Z = C;
            }

            double it = 0;

            while (it < g_fractal().params.iteration_limit && Z.r * Z.r + Z.i * Z.i < g_fractal().params.escape_limit * g_fractal().params.escape_limit){

                if (g_fractal().params.pre_add){
                    Z += C;
                }

                if (g_fractal().params.square_Z){
                    Z = Z * Z;
                }

                if (g_fractal().params.julia){
                    Z += J;
                }

                if (g_fractal().params.J_multiply){
                    J *= Jx;
                }

                if (g_fractal().params.mid_add){
                    Z += C;
                }

                if (g_fractal().params.sponge){
                    Z += complex(g_fractal().params.sponginess) / Z;
                }

                if (g_fractal().params.box_reflect){
                    if (Z.r > g_fractal().params.box_reflect_scale){
                        Z.r = g_fractal().params.box_reflect_scale * 2 - Z.r;
                    } else if (Z.r < -g_fractal().params.box_reflect_scale){
                        Z.r = g_fractal().params.box_reflect_scale * -2 - Z.r;
                    }

                    if (Z.i > g_fractal().params.box_reflect_scale){
                        Z.i = g_fractal().params.box_reflect_scale * 2 - Z.i;
                    } else if (Z.i < -g_fractal().params.box_reflect_scale){
                        Z.i = g_fractal().params.box_reflect_scale * -2 - Z.i;
                    }
                }

                if (g_fractal().params.ring_reflect){
                    double abssqr = (Z.r * Z.r + Z.i * Z.i) * g_fractal().params.ring_reflect_scale * g_fractal().params.ring_reflect_scale;
                    if (sqrt(abssqr) < 0.25){
                        Z *= 4 * g_fractal().params.ring_reflect_scale;
                    } else if (abssqr < 1){
                        Z /= abssqr;
                    }
                }

                if (g_fractal().params.post_add){
                    Z += C;
                }

                if (g_fractal().params.Z_multiply){
                    Z *= Zx;
                }

                if (g_fractal().params.C_multiply){
                    C *= Cx;
                }

                it += 1;
            }

            if (it < g_fractal().params.iteration_limit){
                it += (1 - (log(log(Z.abs())) / log(2)));
            }

            return g_fractal().params.gradient.getColorAt(it / g_fractal().params.iteration_limit).toInt();
        }
    };
    std::vector<std::unique_ptr<Worker>> workers;
};

Fractal g_fractal_instance = {};

Fractal& g_fractal() {
    return g_fractal_instance;
}