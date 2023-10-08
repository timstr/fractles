#pragma once

#include <drawing.h>
#include <image.h>

struct TextRenderer {
    public:
    int frame_w = 0, frame_h;
    int xframes = 0, yframes = 0;

    void loadFont(std::string path, int _frame_w, int _frame_h, int _xframes, int _yframes){
        spritesheet.load(path);
        frame_w = _frame_w;
        frame_h = _frame_h;
        xframes = _xframes;
        yframes = _yframes;
    }

    void writeToTexture(std::string text, Image *image, int max_width = -1, int max_height = -1, Rect *char_rect = nullptr, int char_index = -1){
        if (text.size() == 0){
            image->resize(max_width > 0 ? max_width : frame_w, max_height > 0 ? max_height : frame_h);
            if (char_rect){
                *char_rect = Rect(image->size());
            }
            return;
        }

        int width = 0, height = 0;
        int nchars = text.size();
        bool keep_writing = true;
        int posx = 0, posy = 0;
        int charwidth, lpad;

        for (int i = 0; i < nchars && keep_writing; i++){
            getCharDimensions(text[i], lpad, charwidth);
            if (max_width != -1 && (posx - lpad + 2 + charwidth > max_width)){
                posx = 0;
                posy += frame_h;
            }
            posx += 2 - lpad;

            width = std::max(width, posx + charwidth);
            height = posy + frame_h;

            posx += charwidth;

            if (max_height != -1 && (posy >= max_height)){
                keep_writing = false;
            }
        }
        width += 2;
        if (max_width != -1){
            width = max_width;
        }
        if (max_height != -1){
            height = max_height;
        }

        keep_writing = true;
        posx = 0;
        posy = 0;
        image->resize(width, height);
        for (int i = 0; i < nchars && keep_writing; i++){
            getCharDimensions(text[i], lpad, charwidth);
            if (posx - lpad + 2 + charwidth > width){
                posx = 0;
                posy += frame_h;
            }
            posx += 2 - lpad;
            if (char_rect && char_index == i){
                char_rect->x = posx + 1;
                char_rect->y = posy;
                char_rect->w = charwidth + 1;
                char_rect->h = frame_h;
            }
            blitChar(text[i], image, posx, posy);
            posx += charwidth;

            if (posy >= height){
                keep_writing = false;
            }
        }
        if (char_rect && char_index == nchars){
            if (nchars > 0){
                posx += 2;
            }
            char_rect->x = posx;
            char_rect->y = posy;
            char_rect->w = frame_w;
            char_rect->h = frame_h;
        }
    }
    
    private:
    Image spritesheet;

    void blitChar(char _char, Image *image, int _x, int _y){
        int index = getCharIndex(_char);
        int charx = index % xframes;
        int chary = index / xframes;
        Uint32 pixel;
        for (int ax = 0; ax < frame_w; ax++){
            for (int ay = 0; ay < frame_h; ay++){
                pixel = spritesheet.getPixel(charx * frame_w + ax, chary * frame_h + ay);
                if (Color(pixel).getA() > 0.1){
                    image->setPixel(_x + ax, _y + ay, pixel);
                }
            }
        }
    }

    void getCharDimensions(char _char, int &lpad, int &width){
        if (_char == ' '){
            lpad = 0;
            width = frame_w;
            return;
        }

        lpad = 0;
        int index = getCharIndex(_char);
        int charx = index % xframes;
        int chary = index / xframes;
        
        bool clear = true;
        int minx;
        for (minx = 0; minx < frame_w && clear; minx++){
            for (int y = 0; y < frame_h && clear; y++){
                clear = Color(spritesheet.getPixel(charx * frame_w + minx, chary * frame_h + y)).getA() < 0.1;
            }
        }
        lpad = minx - 1;

        clear = true;
        int maxx;
        for (maxx = frame_w - 1; maxx >= 0 && clear; maxx -= 1){
            for (int y = 0; y < frame_h && clear; y++){
                clear = Color(spritesheet.getPixel(charx * frame_w + maxx, chary * frame_h + y)).getA() < 0.1;
            }
        }

        width = maxx + 1;
    }

    static std::string alphabet;

    int getCharIndex(char _char){
        return(alphabet.find(_char) + 1);
    }
};

std::string TextRenderer::alphabet = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`~!@#$%^&*()-=_+[]\\/{}|;':\"<>?.,";