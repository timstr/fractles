#pragma once

#include "parameditor.h"
#include "rendering.h"

struct FractalWindow : GUI::Window {
    FractalWindow(){
        rect = Rect(1000, 700);

        Fractal::Parameters& params = g_fractal().params;

        int y = 10;

        addElement(Pos(10, y), showhidebtn = new ShowHideBtn(*this));
        addElement(Pos(100, y), save_btn = new SaveBtn());
        addElement(Pos(190, y), size_editor = new SizeEditor(*this));
        y += 30;

        addElement(Pos(10, y), new Switch(params.square_Z));
        addElement(Pos(60, y), new Switch(params.Z_init));
        y += 30;

        if (!params.texture.has_value()) {
            gradient_editor = new GradientEditor(params.gradient);
            addElement(Pos(270, y), gradient_editor);
        }

        addElement(Pos(10, y + 15), new Switch(params.julia));
        addElement(Pos(60, y), new Slider1D(params.julia_r, -2, 2, 200, "Julia R"));
        addElement(Pos(60, y + 30), new Slider1D(params.julia_i, -2, 2, 200, "Julia I"));
        y += 60;

        addElement(Pos(10, y + 15), new Switch(params.J_multiply));
        addElement(Pos(60, y), new Slider1D(params.J_multiplier_r, -2, 2, 200, "Julia multi R"));
        addElement(Pos(60, y + 30), new Slider1D(params.J_multiplier_i, -2, 2, 200, "Julia multi I"));
        y += 60;

        addElement(Pos(60, y), new Switch(params.pre_add));
        addElement(Pos(110, y), new Switch(params.mid_add));
        addElement(Pos(160, y), new Switch(params.post_add));
        y += 30;

        addElement(Pos(10, y), new Switch(params.sponge));
        addElement(Pos(60, y), new Slider1D(params.sponginess, -2, 2, 200, "Sponginess"));
        y += 30;

        addElement(Pos(10, y), new Switch(params.box_reflect));
        addElement(Pos(60, y), new Slider1D(params.box_reflect_scale, -2, 2, 200, "Box Reflect"));
        y += 30;

        addElement(Pos(10, y), new Switch(params.ring_reflect));
        addElement(Pos(60, y), new Slider1D(params.ring_reflect_scale, -2, 2, 200, "Ring Reflect"));
        y += 30;

        addElement(Pos(10, y + 15), new Switch(params.Z_multiply));
        addElement(Pos(60, y), new Slider1D(params.Z_multiplier_r, -2, 2, 200, "Z multi R"));
        addElement(Pos(60, y + 30), new Slider1D(params.Z_multiplier_i, -2, 2, 200, "Z multi I"));
        y += 60;

        addElement(Pos(10, y + 15), new Switch(params.C_multiply));
        addElement(Pos(60, y), new Slider1D(params.C_multiplier_r, -2, 2, 200, "C multi R"));
        addElement(Pos(60, y + 30), new Slider1D(params.C_multiplier_i, -2, 2, 200, "C multi I"));
        y += 60;

        addElement(Pos(10, y + 15), new Switch(params.shade));
        addElement(Pos(60, y), new Slider1D(params.highlight_strength, 0, 1, 200, "Highlight"));
        addElement(Pos(60, y + 30), new Slider1D(params.shade_strength, 0, 1, 200, "Shade"));
        y += 60;

        if (params.texture.has_value()) {
            addElement(Pos(60, y), new Slider1D(params.texture_x_offset, 0, 1, 95, "x offset"));
            addElement(Pos(165, y), new Slider1D(params.texture_x_scale, 0, 5, 95, "x scale"));
            addElement(Pos(60, y + 30), new Slider1D(params.texture_y_offset, 0, 1, 95, "y offset"));
            addElement(Pos(165, y + 30), new Slider1D(params.texture_y_scale, 0, 5, 95, "y scale"));
        }
    }

    struct Grabber : GUI::Window {
        Grabber(FractalWindow& _fractal_window) : fractal_window(_fractal_window){
            start_x = g_fractal().params.x_offset;
            start_y = g_fractal().params.y_offset;
            start_pos = rect.pos();
            old_pos = rect.pos();
        }
        void render(Pos /*offset*/) override {

        }
        void onDrag() override {
            if (!(old_pos == rect.pos())){
                Pos diff = rect.pos() - start_pos;
                g_fractal().params.x_offset = start_x - (diff.x / (double)std::min(g_fractal().w(), g_fractal().h()) / g_fractal().params.magnification);
                g_fractal().params.y_offset = start_y - (diff.y / (double)std::min(g_fractal().w(), g_fractal().h()) / g_fractal().params.magnification);
                g_fractal().rerender = true;
            }
            old_pos = rect.pos();
        }
        void onRightRelease() override {
            close();
        }
        FractalWindow& fractal_window;
        Pos start_pos, old_pos;
        double start_x, start_y;
    };

    void onRightClick(int clicks) override {
        if (gradient_editor) {
            gradient_editor->color_editor->hide();
        }
        if (clicks == 1){
            Grabber* grabber = new Grabber(*this);
            addElement(grabber);
            grabber->startDrag();
        } else if (clicks >= 2){
            g_fractal().params.magnification /= 1.1;
            g_fractal().rerender = true;
        }
    }

    void onLeftClick(int clicks){
        if (gradient_editor) {
            gradient_editor->color_editor->hide();
        }
        if (clicks == 1){
            // TODO: add a new colour stripe to the gradient where the fractal was clicked
        } else if (clicks >= 2){
            g_fractal().params.magnification *= 1.1;
            g_fractal().rerender = true;
        }
    }

    void onKeyDown(SDL_Scancode key) override {
        switch (key){
            case SDL_SCANCODE_ESCAPE:
                g_fractal().abortRender();
                break;
            case SDL_SCANCODE_R:
                if (gradient_editor) {
                    gradient_editor->randomize(3 + rand() % 27);
                    g_fractal().rerender = true;
                }
                break;
            case SDL_SCANCODE_TAB:
                actual_size = !actual_size;
                break;
            case SDL_SCANCODE_SPACE:
                toggle_tool_visibility();
                break;
            case SDL_SCANCODE_RETURN:
                g_fractal().save();
                break;
            default:
                break;
        }
    }

    void render(Pos offset) override {
        if (keyDown(SDL_SCANCODE_W)){
            g_fractal().params.y_offset -= 0.01 / g_fractal().params.magnification;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_A)){
            g_fractal().params.x_offset -= 0.01 / g_fractal().params.magnification;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_S)){
            g_fractal().params.y_offset += 0.01 / g_fractal().params.magnification;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_D)){
            g_fractal().params.x_offset += 0.01 / g_fractal().params.magnification;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_UP)){
            g_fractal().params.magnification *= 1.05;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_DOWN)){
            g_fractal().params.magnification /= 1.05;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_Q)){
            g_fractal().params.rotation -= PI * 0.005;
            g_fractal().rerender = true;
        }
        if (keyDown(SDL_SCANCODE_E)){
            g_fractal().params.rotation += PI * 0.005;
            g_fractal().rerender = true;
        }

        if (!(GUI::getScreenSize() == Pos(g_fractal().w(), g_fractal().h()))){
            rect.w = GUI::getScreenSize().x;
            rect.h = GUI::getScreenSize().y;
            if (actual_size){
                g_fractal().resize(rect.w, rect.h);
                g_fractal().rerender = true;
            }
        }

        if (g_fractal().rerender){
            g_fractal().beginRender();
            g_fractal().rerender = false;
        }
        if (g_fractal().isComplete() && !g_fractal().isAntiAliased()){
            g_fractal().beginAntiAlias();
        }

        g_fractal().render();

        renderElements(offset);
    }

    void show(){
        for (size_t i = 0; i < elements.size(); i++){
            elements[i]->visible = true;
        }
        showhidebtn->show = true;
        showhidebtn->updateCaption();
    }

    void hide(){
        for (size_t i = 0; i < elements.size(); i++){
            elements[i]->visible = false;
        }
        showhidebtn->visible = true;
        showhidebtn->show = false;
        showhidebtn->updateCaption();
    }

    void toggle_tool_visibility() {
        if (showhidebtn->show) {
            hide();
        } else {
            show();
        }
    }

    bool actual_size = true;

    GradientEditor* gradient_editor = nullptr;

    struct ShowHideBtn : GUI::Window {
        ShowHideBtn(FractalWindow& _fractal_window) : fractal_window(_fractal_window) {
            addElement(caption = new GUI::Text("Hide Tools"));
            rect.w = caption->rect.w;
            rect.h = 20;
        }
        void onLeftClick(int /*clicks*/) override {
            show = !show;
            if (show){
                fractal_window.show();
            } else {
                fractal_window.hide();
            }

        }
        void updateCaption() {
            caption->setText(show ? "Hide Tools" : "Show Tools");
            rect.w = caption->rect.w;
        }
        bool show = true;
        GUI::Text* caption = nullptr;
        FractalWindow& fractal_window;
    } *showhidebtn = nullptr;

    struct SaveBtn : GUI::Window {
        SaveBtn(){
            GUI::Text* caption = new GUI::Text("Save");
            addElement(caption);
            rect = Rect(caption->rect.size());
        }
        void onLeftClick(int /*clicks*/) override {
            g_fractal().save();
        }
        void render(Pos offset){
            if (g_fractal().isComplete()){
                drawColor(0xFF80FF80);
                drawRect(rect + offset);
            } else {
                int x = g_fractal().getProgress() * rect.w;
                drawColor(0xFF404040);
                drawRect(Rect(rect.x + x, rect.y, rect.w - x, rect.h));
                drawColor(0xFF808000);
                drawRect(Rect(rect.x, rect.y, x, rect.h));
            }

            renderElements(offset);
        }
    } *save_btn = nullptr;

    struct SizeEditor : GUI::Window {
        SizeEditor(FractalWindow& _fractal_window) : fractal_window(_fractal_window) {
            addElement(Pos(0, 0), width_box = new WidthTextEntry(fractal_window));
            addElement(Pos(110, 0), height_box = new HeightTextEntry(fractal_window));
            addElement(Pos(220, 0), actual_size_btn = new ActualSizeBtn(fractal_window));
            old_width = g_fractal().w();
            old_height = g_fractal().h();
            width_box->setText(std::to_string(g_fractal().w()));
            height_box->setText(std::to_string(g_fractal().h()));
        }
        void render(Pos offset) override {
            if (g_fractal().w() != old_width){
                width_box->stopTextEntry();
                width_box->setText(std::to_string(g_fractal().w()));
            }
            if (g_fractal().h() != old_height){
                height_box->stopTextEntry();
                height_box->setText(std::to_string(g_fractal().h()));
            }
            old_width = g_fractal().w();
            old_height = g_fractal().h();
            renderElements(offset);
        }
        FractalWindow& fractal_window;
        struct WidthTextEntry : GUI::TextEntry {
            FractalWindow& fractal_window;
            WidthTextEntry(FractalWindow& _fractal_window) : fractal_window(_fractal_window){

            }
            void onLeftClick(int /*clicks*/) override {
                beginTextEntry();
            }
            void render(Pos offset) override {
                drawColor(0xFFAAFFAA);
                drawRect(rect + offset);
                GUI::TextEntry::render(offset);
            }
            void onReturn(std::string text) override {
                if (!text.empty()){

                    std::stringstream stream;
                    stream.str(text);

                    int w;

                    stream >> w;

                    if (stream){
                        fractal_window.actual_size = false;
                        g_fractal().resize(w, g_fractal().h());
                        g_fractal().beginRender();
                        return;
                    }
                }
            }
        }* width_box = nullptr;
        struct HeightTextEntry : GUI::TextEntry {
            FractalWindow& fractal_window;
            HeightTextEntry(FractalWindow& _fractal_window) : fractal_window(_fractal_window){

            }
            void onLeftClick(int /*clicks*/) override {
                beginTextEntry();
            }
            void render(Pos offset) override {
                drawColor(0xFFAAFFAA);
                drawRect(rect + offset);
                GUI::TextEntry::render(offset);
            }
            void onReturn(std::string text) override {
                if (!text.empty()){

                    std::stringstream stream;
                    stream.str(text);

                    int h;

                    stream >> h;

                    if (stream){
                        fractal_window.actual_size = false;
                        g_fractal().resize(g_fractal().w(), h);
                        g_fractal().beginRender();
                        return;
                    }
                }
            }
        }* height_box = nullptr;
        size_t old_width, old_height;
        struct ActualSizeBtn : GUI::Window {
            ActualSizeBtn(FractalWindow& _fractal_window) : fractal_window(_fractal_window) {
                GUI::Text* caption = new GUI::Text("Actual Size");
                addElement(caption);
                rect = Rect(caption->rect.size());
            }
            void onLeftClick(int /*clicks*/){
                fractal_window.actual_size = !fractal_window.actual_size;
            }
            void render(Pos offset) override {
                drawColor(fractal_window.actual_size ? 0xFFFFFFFF : 0xFF808080);
                drawRect(rect + offset);
                renderElements(offset);
            }
            FractalWindow& fractal_window;
        }* actual_size_btn = nullptr;
    } *size_editor = nullptr;
};
