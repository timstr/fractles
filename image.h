#pragma once

#include <iostream>
#include <string>
#include <drawing.h>
#include <SDL_image.h>

struct Image {
    Image(){

    }

    Image(unsigned int width, unsigned int height){
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        locked = false;

        w = width;
        h = height;

        for (int x = 0; x < w; x++){
            for (int y = 0; y < h; y++){
                setPixel(x, y, 0);
            }
        }
    }

    Image(std::string path){
        load(path);
    }

    Image(const Image& img){
        copyFrom(img);
    }

    void operator=(const Image& img){
        copyFrom(img);
    }

    ~Image(){
        cleanup();
    }

    void resize(unsigned int width, unsigned int height){
        cleanup();

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        w = width;
        h = height;

        for (int x = 0; x < w; x++){
            for (int y = 0; y < h; y++){
                setPixel(x, y, 0);
            }
        }
    }

    void setPixel(int x, int y, Uint32 color){
        if (texture){
            lock();
            if (x >= 0 && x < w && y >= 0 && y < h){
                pixels[(y * pitch) + x] = color;
            }
        }
    }

    Color getPixel(int x, int y) const {
        if (texture){
            lock();
            if (x >= 0 && x < w && y >= 0 && y < h){
                return Color(pixels[(y * pitch) + x]);
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
    
    void render(int x, int y){
        SDL_Rect _dst;
        _dst.x = x;
        _dst.y = y;
        _dst.w = w;
        _dst.h = h;

        if (texture){
            unlock();
            SDL_RenderCopy(renderer, texture, nullptr, &_dst);
        } else {
            _dst.w = 50;
            _dst.h = 50;
            SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
            SDL_RenderFillRect(renderer, &_dst);
        }
    }
    
    void render(Pos pos){
        SDL_Rect _dst;
        _dst.x = pos.x;
        _dst.y = pos.y;
        _dst.w = w;
        _dst.h = h;

        if (texture){
            unlock();
            SDL_RenderCopy(renderer, texture, nullptr, &_dst);
        } else {
            _dst.w = 50;
            _dst.h = 50;
            SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
            SDL_RenderFillRect(renderer, &_dst);
        }
    }
    
    void render(Rect& location){
        SDL_Rect _dst;
        _dst.x = location.x;
        _dst.y = location.y;
        _dst.w = location.w;
        _dst.h = location.h;

        if (texture){
            unlock();
            SDL_RenderCopy(renderer, texture, nullptr, &_dst);
        } else {
            SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
            SDL_RenderFillRect(renderer, &_dst);
        }
    }
    
    void render(Rect& location, Rect& slice){
        SDL_Rect _dst;
        _dst.x = location.x;
        _dst.y = location.y;
        _dst.w = location.w;
        _dst.h = location.h;

        SDL_Rect _src;
        _src.x = slice.x;
        _src.y = slice.y;
        _src.w = slice.w;
        _src.h = slice.h;

        if (texture){
            unlock();
            SDL_RenderCopy(renderer, texture, &_src, &_dst);
        } else {
            SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, rand() % 256);
            SDL_RenderFillRect(renderer, &_dst);
        }
    }

    void load(std::string path){
        cleanup();

        SDL_Surface* loaded = IMG_Load(path.c_str());

        if (loaded == nullptr){
            std::cout << "Failed to load image from \"" << path << "\"\n";
            return;
        }

        SDL_PixelFormat format = *SDL_GetWindowSurface(window)->format;

        format.format = SDL_PIXELFORMAT_ARGB8888;
        format.Amask = 0xFF000000;
        format.Rmask = 0x00FF0000;
        format.Gmask = 0x0000FF00;
        format.Bmask = 0x000000FF;

        SDL_Surface* surface = SDL_ConvertSurface(loaded, &format, 0);

        if (surface == nullptr){
            std::cout << "Failed to load image from \"" << path << "\"\n";
            return;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, surface->w, surface->h);

        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

        w = surface->w;
        h = surface->h;

        Uint32* pdata = (Uint32*)surface->pixels;
        int p = surface->pitch / sizeof(Uint32);
        for (int x = 0; x < w; x++){
            for (int y = 0; y < h; y++){
                setPixel(x, y, pdata[(y * p) + x]);
            }
        }

        if (texture == nullptr){
            std::cout << "Failed to load image from \"" << path << "\"\n";
            return;
        }

        SDL_FreeSurface(surface);
    }

    void save(std::string path){
        unlock();

        if (!texture){
            throw;
        }

        SDL_Surface* surface = SDL_CreateRGBSurface(0, w, h, 32, 0xFF0000, 0xFF00, 0xFF, 0xFF000000);

        for (int x = 0; x < w; x++){
            for (int y = 0; y < h; y++){
                ((Uint32*)surface->pixels)[(y * surface->pitch) + x] = getPixel(x, y).toInt();
            }
        }

        IMG_SavePNG(surface, path.c_str());

        SDL_FreeSurface(surface);
    }

    int width(){
        return w;
    }

    int height(){
        return h;
    }

    Pos size(){
        return Pos(w, h);
    }

    private:
    int w = 0;
    int h = 0;
    SDL_Texture* texture = nullptr;
    mutable bool locked = false;
    mutable Uint32* pixels = nullptr;
    mutable int pitch = 0;

    void lock() const {
        if (!locked && texture){
            SDL_LockTexture(texture, nullptr, (void**)&pixels, &pitch);
            if (pixels != nullptr){
                locked = true;
                pitch /= sizeof(Uint32);
            } else {
                std::cout << "Error! Null pointer to texture pixel data.\n";
            }
        }
    }

    void unlock(){
        if (locked && texture){
            SDL_UnlockTexture(texture);
            locked = false;
            pixels = nullptr;
        }
    }

    void cleanup(){
        unlock();

        if (texture){
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }
        w = 0;
        h = 0;
        pixels = nullptr;
        locked = false;
    }

    void copyFrom(const Image& img){
        cleanup();
        if (img.texture){
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, img.w, img.h);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

            w = img.w;
            h = img.h;

            for (int x = 0; x < w; x++){
                for (int y = 0; y < h; y++){
                    setPixel(x, y, img.getPixel(x, y));
                }
            }
        }
    }
};