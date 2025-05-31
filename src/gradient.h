#ifndef _GRADIENT_
#define _GRADIENT_

//#include <texture.h>
#include <mycolors.h>
#include <mymaths.h>
#include <vector>

struct Gradient {
    struct ColorNode {
        Color color;
        double pos = 0;
    };

    ColorNode* addNode(Color color, double pos){
        ColorNode *node = new ColorNode();
        node->color = color;
        node->pos = pos;
        nodes.push_back(node);
        return node;
    }
    void removeNode(ColorNode* node){
        for (size_t i = 0; i < nodes.size(); i++){
            if (nodes[i] == node){
                nodes.erase(nodes.begin() + i);
                return;
            }
        }
        throw;
    }

    Color getColorAt(double pos){
        ColorNode *left = nullptr, *right = nullptr;
        for (size_t i = 0; i < nodes.size(); i++){
            if (nodes[i]->pos <= pos){
                if (left){
                    if (nodes[i]->pos > left->pos){
                        left = nodes[i];
                    }
                } else {
                    left = nodes[i];
                }
            }
            if (nodes[i]->pos > pos){
                if (right){
                    if (nodes[i]->pos < right->pos){
                        right = nodes[i];
                    }
                } else {
                    right = nodes[i];
                }
            }
        }
        Color lcolor = left ? left->color : (right ? right->color : Color(0xFF000000));
        Color rcolor = right ? right->color : (left ? left->color : Color(0xFF000000));
        double lpos = left ? left->pos : 0.0;
        double rpos = right ? right->pos : 1.0;
        if (lpos == rpos) {
            return lcolor;
        } else {
            return transition(lcolor, rcolor, (pos - lpos) / (rpos - lpos));
        }
    }

    void randomize(size_t n_colors){
        clear();
        for (size_t i = 0; i < n_colors; i++){
            addNode(ColorHSVA(dRand(1), dRand(1), dRand(1), 1.0), dRand(1));
        }
    }

    void clear(){
        for (size_t i = 0; i < nodes.size(); i++){
            delete nodes[i];
        }
        nodes.clear();
    }

    std::vector<ColorNode*> nodes;
};

/*struct ColorPos: public Color {
    double pos;

    ColorPos(){
        setColor(0xFF000000);
        pos = 0;
    }

    ColorPos(double _pos, Color _color){
        setColor(_color);
        pos = _pos;
    }
};

struct Gradient {
    public:

    void randomize(){
        gradient.clear();
        ColorPos colors[3] = {ColorPos(), ColorPos(), ColorPos()};
        double tpos;
        int c, n;

        switch (rand() % 5){
            case 0:
                n = 5 + (rand() % 45);
                for (int i = 0; i < n; i++){
                    addColor(ColorPos(pow(dRand(1), 2), ColorRGBA(dRand(1), dRand(1), dRand(1))));
                }
            break;
            case 1:
                do {
                    for (int i = 0; i < 3; i++){
                        colors[0] = ColorPos(0, ColorRGBA(dRand(1), dRand(1), dRand(1)));
                    }
                } while (getColorDiffRGB(colors[0], colors[1]) < 0.5 && getColorDiffRGB(colors[0], colors[2]) < 0.5 && getColorDiffRGB(colors[1], colors[2]) < 0.5);

                n = 5 + (rand() % 45);
                for (int i = 0; i < n; i++){
                    c = rand() % 3;
                    addColor(ColorPos(colors[c].pos, Color(colors[c].getR() + pnRand(0.1), colors[c].getG() + pnRand(0.1), colors[c].getB() + pnRand(0.1), colors[c].getA() + pnRand(0.1))));
                }
            break;
            case 2:
                n = 5 + (rand() % 20);
                for (int i = 0; i < n; i++){
                    tpos = pow(dRand(1), 2);
                    addColor(ColorPos(tpos, ColorRGBA(0, 0, 0, 1)));
                    addColor(ColorPos(min(tpos + 0.00001, 1), ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                    addColor(ColorPos(min(tpos + 0.00049, 1), ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                    addColor(ColorPos(min(tpos + 0.00050, 1), ColorRGBA(0, 0, 0, 1)));
                }
            break;
            case 3:
                n = 5 + (rand() % 20);
                for (int i = 0; i < n; i++){
                    tpos = pow(dRand(1), 2);
                    addColor(ColorPos(tpos, ColorRGBA(0, 0, 0, 1)));
                    addColor(ColorPos(min(tpos + 0.00001, 1), ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                    addColor(ColorPos(min(tpos + 0.00049, 1), ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                }
            break;
            case 4:
                n = 5 + (rand() % 20);
                for (int i = 0; i < n; i++){
                    tpos = pow(dRand(1), 2);
                    addColor(ColorPos(tpos, ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                    addColor(ColorPos(min(tpos + 0.00001, 1), ColorRGBA(dRand(1), dRand(1), dRand(1), 1)));
                }
            break;
        }
    }

    Gradient(){
        gradient.clear();
        gradient.push_back(ColorPos(0, Color(0, 0, 0, 1)));
    }

    int getPrevColor(double pos, bool skip = false){
        int prev = -1;
        double prevpos = 0;

        for (int i = 0; i < gradient.size(); i++){
            if (skip){
                if (gradient[i].pos > prevpos && gradient[i].pos < pos){
                    prevpos = gradient[i].pos;
                    prev = i;
                }
            } else {
                if (gradient[i].pos > prevpos && gradient[i].pos <= pos){
                    prevpos = gradient[i].pos;
                    prev = i;
                }
            }
        }

        return(prev);
    }

    int getNextColor(double pos, bool skip = false){
        int next = -1;
        double nextpos = 1;

        if (next == -1){
            for (int i = 0; i < gradient.size(); i++){
                if (skip){
                    if (gradient[i].pos < nextpos && gradient[i].pos > pos){
                        nextpos = gradient[i].pos;
                        next = i;
                    }
                } else {
                    if (gradient[i].pos < nextpos && gradient[i].pos >= pos){
                        nextpos = gradient[i].pos;
                        next = i;
                    }
                }
            }
        }

        return(next);
    }

    Color colorAt(double pos){
        int prev = getPrevColor(pos), next = getNextColor(pos);

        if (prev == -1){
            if (next == -1){
                return(0xFF000000);
            } else {
                return(gradient[next]);
            }
        } else if (next == -1){
            return(gradient[prev]);
        } else {
            return(transition(gradient[prev], gradient[next], (pos - gradient[prev].pos) / (gradient[next].pos - gradient[prev].pos)));
        }
    }

    void render(Texture *texture, SDL_Renderer *renderer, int width, int height){
        texture->build(renderer, width, height);

        Uint32 tcolor;

        for (int x = 0; x < width; x++){
            tcolor = colorAt(x / (double)width);
            for (int y = 0; y < height; y++){
                texture->putPixel(x, y, tcolor);
            }
        }
    }

    void addColor(ColorPos _color){
        gradient.push_back(_color);
    }

    void setColor(int index, ColorPos _color){
        if (index < (int)gradient.size() && index >= 0){
            gradient[index] = _color;
        }
    }

    void removeColor(int index){
        if (index < gradient.size() && index >= 0){
            gradient.erase(gradient.begin() + index);
        }
    }

    private:

    std::vector<ColorPos> gradient;
};*/
    
#endif