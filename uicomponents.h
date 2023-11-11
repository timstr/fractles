#pragma once

struct Slider1D : GUI::Window {
    Slider1D(double& target, double _min, double _max, int width, std::string _label) : value(target) {
        min = _min;
        max = _max;
        rect = Rect(width, 20);
        addElement(label = new GUI::Text(_label));
        addElement(handle = new Handle(*this));
    }
    void onLeftClick(int clicks) override {
        if (clicks == 2){
            value = 0.0;
            g_fractal().rerender = true;
        } else {
            handle->rect.x = std::min(std::max(localMousePos().x - handle->rect.w / 2, 0), rect.w);
            handle->startNormalDrag();
        }
    }
    void onRightClick(int clicks) override {
        if (clicks == 2){
            value = 1.0;
            g_fractal().rerender = true;
        } else {
            handle->rect.x = std::min(std::max(localMousePos().x - handle->rect.w / 2, 0), rect.w);
            handle->startFineDrag();
        }
    }
    void render(Pos offset) override {
        drawColor(0xFFFFFFFF);
        drawRect(rect + offset);
        renderElements(offset);
    }

    private:
    double min = -1, max = 1;
    double& value;

    GUI::Text* label;

    struct Handle : GUI::Window {
        Handle(Slider1D& _slider) : slider(_slider){
            rect = Rect(20, 20);
        }
        void startNormalDrag(){
            fine = false;
            startDrag();
        }
        void startFineDrag(){
            fine = true;
            start_val = slider.value;
            start_x = rect.x;
            startDrag();
        }
        void onLeftClick(int clicks) override {
            if (clicks == 2){
                slider.value = 0;
                g_fractal().rerender = true;
            } else {
                startNormalDrag();
            }
        }
        void onRightClick(int clicks) override {
            if (clicks == 2){
                slider.value = 1;
                g_fractal().rerender = true;
            } else {
                startFineDrag();
            }
        }
        void onDrag() override {
            if (fine){
                rect.y = 0;
                int x = rect.x;
                if (x != old_x){
                    double diff = (rect.x - start_x) / 100.0 / rect.w / (slider.max - slider.min);
                    slider.value = std::min(std::max(start_val + diff, slider.min), slider.max);
                    rect.x = (slider.value - slider.min) / (slider.max - slider.min) * slider.rect.w;
                    g_fractal().rerender = true;
                }
                old_x = x;
            } else {
                rect.x = std::min(std::max(rect.x, 0), slider.rect.w - rect.w);
                rect.y = 0;
                if (rect.x != old_x){
                    slider.value = rect.x * (slider.max - slider.min) / (slider.rect.w - rect.w) + slider.min;
                    g_fractal().rerender = true;
                }
                old_x = rect.x;
            }
        }
        void render(Pos offset) override {
            rect.x = (slider.value - slider.min) / (slider.max - slider.min) * (slider.rect.w - rect.w);
            drawColor(0xFF808080);
            drawRect(rect + offset);
        }
        bool fine = false;
        private:
        Slider1D& slider;
        int old_x = 0;
        double start_val;
        int start_x;
    }* handle = nullptr;
    friend struct Handle;
};

struct Switch : GUI::Window {
    bool& value;
    Switch(bool& target) : value(target) {
        rect = Rect(40, 20);
    }
    void onLeftClick(int /*clicks*/) override {
        value = !value;
        g_fractal().rerender = true;
    }
    void render(Pos offset) override {
        drawColor(value ? 0xFFFFFFFF : 0xFF000000);
        drawRect(rect + offset);
    }
};