#pragma once

#include <mymaths.h>
#include <drawing.h>
#include <textrenderer.h>
#include <image.h>
#include <map>

namespace GUI {

    struct Element;
    struct Container;
    struct Window;
    struct Text;
    struct TextEntry;

    double getProgramTime();

    struct GUIContext {
        private:
        bool quit;

        Pos screensize;
        Pos mouse_pos = Pos();
        Pos drag_offset = Pos();

        Window* dragging_window = nullptr;
        Window* current_window = nullptr;
        TextEntry* text_entry = nullptr;

        Uint32 ticks;
        double program_time;

        const Uint8 *keystates = SDL_GetKeyboardState(nullptr);
        std::map<std::pair<SDL_Scancode, std::vector<SDL_Scancode>>, void (*)(const Uint8*)> commands;
        bool (*quit_handler)() = nullptr;

        void focusTo(Window* window);

        TextRenderer text_renderer;

        friend void addKeyboardCommand(SDL_Scancode, void (*)(const Uint8*));
        friend void addKeyboardCommand(SDL_Scancode, std::vector<SDL_Scancode>, void (*)(const Uint8*));
        friend void setQuitHandler(bool (*)());
        friend double getProgramTime();
        friend Pos getScreenSize();
        friend void init(Pos, std::string);
        friend void run();
        friend class Window;
        friend class Element;
        friend class TextEntry;
        friend class Text;
        friend class GUIRootWindow;
    } Context;

    struct Element {
        bool visible = true;
        Rect rect = Rect(0, 0, 50, 50);

        virtual ~Element();
        void close(){
            delete this;
        }

        virtual void render(Pos offset){//TODO: make GUI rendering happen without having to change offset each call
            drawColor(0xFF000060);
            drawRect(rect + offset);
            drawColor(0xFF000000);
            drawRectBorder(rect + offset);
        }
        
        virtual Element* elementAt(Pos pos){
            if (rect.hit(pos) && visible){
                return this;
            } else {
                return nullptr;
            }
        }
        virtual Window* windowAt(Pos /*pos*/){
            return nullptr;
        }

        Pos localMousePos(){
            Pos pos = Context.mouse_pos;
            Element *element = this;
            while (element != nullptr){
                pos -= element->rect.pos();
                element = (Element*)element->parent;
            }
            return(pos);
        }
        Pos rootPos(){
            Pos pos = Pos();
            Element *element = this;
            while (element != nullptr){
                pos += element->rect.pos();
                element = (Element*)element->parent;
            }
            return pos;
        }

        private:
        Window* parent = nullptr;

        friend Window;
        friend class GUIContext;
    };

    struct Window : Element {
        bool disabled = false;

        ~Window();

        virtual void onLeftClick(int /*clicks*/){

        }
        virtual void onLeftRelease(){
            
        }
        virtual void onRightClick(int /*clicks*/){

        }
        virtual void onRightRelease(){

        }

        void startDrag(){
            Context.focusTo(this);
            Context.drag_offset = Context.mouse_pos - rect.pos();
            Context.dragging_window = this;
        }
        virtual void onDrag(){

        }

        virtual void onHover(){

        }
        
        virtual void onHoverWithElement(Element* /*drag_element*/){

        }
        virtual void onreleaseElement(Element* /*element*/){

        }

        virtual void onFocus(){

        }
        virtual void onLoseFocus(){

        }
        void grabFocus(){
            Context.focusTo(this);
        }

        virtual void onKeyDown(SDL_Scancode /*key*/){

        }
        virtual void onKeyUp(SDL_Scancode /*key*/){

        }
        bool keyDown(SDL_Scancode key){
            if (Context.current_window == this){
                return Context.keystates[key];
            } else {
                return false;
            }
        }

        void addElement(Element *element){
            if (element->parent != nullptr){
                throw;
            }
            element->parent = this;
            elements.insert(elements.begin(), element);
        }
        void addElement(Pos pos, Element *element){
            element->rect.x = pos.x;
            element->rect.y = pos.y;
            addElement(element);
        }
        void releaseElement(Element* element){
            for (size_t i = 0; i < elements.size(); i++){
                if (elements[i] == element){
                    elements[i]->parent = nullptr;
                    elements.erase(elements.begin() + i);
                    return;
                }
            }
            throw;
        }
        void bringToFront(Element *element){
            for (size_t i = 0; i < elements.size(); i++){
                if (elements[i] == element){
                    Element* temp = elements[i];
                    elements.erase(elements.begin() + i);
                    elements.insert(elements.begin(), temp);
                    return;
                }
            }
            throw;
        }
        void clear(){
            while (elements.size() > 0){
                elements[0]->close();
            }
        }

        Window* windowAt(Pos pos){
            if (!visible || disabled){
                return nullptr;
            }

            Window* window = nullptr;
            for (size_t i = 0; i < elements.size() && window == nullptr; i++){
                window = elements[i]->windowAt(pos - rect.pos());
                if (window && window == Context.dragging_window){
                    window = nullptr;
                }
            }
            if (window){
                return window;
            }

            if (rect.hit(pos)){
                return this;
            }

            return nullptr;
        }
        Element* elementAt(Pos pos){
            Element* element = nullptr;
            for (size_t i = 0; i < elements.size() && element == nullptr; i++){
                element = elements[i]->elementAt(pos - rect.pos());
                if (element && element == Context.dragging_window){
                    element = nullptr;
                }
            }
            if (element){
                return element;
            }

            if (rect.hit(pos)){
                return this;
            }

            return nullptr;
        }

        void render(Pos offset){
            drawColor(0xB0B0B0B0);
            drawRect(rect + offset);
            renderElements(offset);
        }
        void renderElements(Pos offset){
            for (int i = elements.size() - 1; i >= 0; i -= 1){
                if (elements[i]->visible){
                    elements[i]->render(offset + Pos(rect.x, rect.y));
                }
            }
        }

        protected:
        std::vector<Element*> elements;

        friend Element;
    };

    Element::~Element(){
        if (parent){
            parent->releaseElement(this);
        }
    }

    void GUIContext::focusTo(Window* window){
        if (Context.current_window != window){
            std::vector<Window*> current_path;
            std::vector<Window*> new_path;

            Window* twindow = Context.current_window;
            while (twindow != nullptr){
                current_path.push_back(twindow);
                twindow = twindow->parent;
            }

            twindow = window;
            while (twindow != nullptr){
                new_path.push_back(twindow);
                twindow = twindow->parent;
            }

            while ((current_path.size() > 0) && (new_path.size() > 0) && (current_path.back() == new_path.back())){
                current_path.pop_back();
                new_path.pop_back();
            }

            while (current_path.size() > 0){
                current_path.front()->onLoseFocus();
                current_path.erase(current_path.begin());
            }

            while (new_path.size() > 0){
                new_path.back()->onFocus();
                new_path.pop_back();
            }

            Context.current_window = window;
        }
    }

    struct Text : Element {
        Text(std::string _text){
            text = _text;
            Context.text_renderer.writeToTexture(text, &image);
            rect.w = image.width();
            rect.h = image.height();
            autosize = true;
        }
        Text(Rect _rect = Rect(), std::string _text = "", bool _autosize = true){
            rect = _rect;
            this->setText(_text);
            autosize = _autosize;
        }

        void setText(std::string _text){
            text = _text;
            redrawText();
        }
        std::string getText(){
            return(text);
        }

        void redrawText(){
            if (autosize){
                Context.text_renderer.writeToTexture(text, &image);
                rect.w = image.width();
                rect.h = image.height();
            } else {
                Context.text_renderer.writeToTexture(text, &image, rect.w, rect.h);
            }
        }

        void render(Pos offset){
            auto r = Rect(rect.pos() + offset, rect.size());
            image.render(r);
        }
        
        protected:
        Image image;
        std::string text;
        bool autosize = true;
    };

    struct TextEntry : Window { //TODO: figure out why deleting everything leaves ghost characters
        bool autosize_x = false;
        bool autosize_y = false;

        TextEntry(Rect _rect = Rect(), std::string _text = "", bool _autosize_x = true, bool _autosize_y = false){
            rect = _rect;
            autosize_x = _autosize_x;
            autosize_y = _autosize_y;
            this->setText(_text);
        }
        ~TextEntry(){
            stopTextEntry();
        }

        void setText(std::string _text){
            text = _text;
            cursor = text.size();
            redrawText();
        }
        std::string getText(){
            return(text);
        }

        void updateTextEntry(std::string text, int /*selection_start*/, int /*selection_length*/){
            setText(text);
            cursor += text.size();
        }

        void type(std::string _text_entered){
            text.insert(cursor, _text_entered);
            cursor += _text_entered.size();
            redrawText();
        }
        void textEditBackspace(){
            if (text.size() > 0 && cursor > 0){
                text.erase(cursor - 1, 1);
                cursor -= 1;
                redrawText();
            }
        }
        void textEditDelete(){
            if (text.size() > 0 && cursor < text.size()){
                text.erase(cursor, 1);
                redrawText();
            }
        }
        void textEditLeft(){
            if (cursor > 0) {
                cursor = cursor - 1;
            }
            redrawText();
        }
        void textEditRight(){
            cursor = cursor + 1;
            if (cursor > text.size()){
                cursor = text.size();
            }
            redrawText();
        }
        void textEditHome(){
            cursor = 0;
            redrawText();
        }
        void textEditEnd(){
            cursor = text.size();
            redrawText();
        }
        void textEditReturn(){
            onReturn(text);
        }
        void beginTextEntry(){
            SDL_StartTextInput();
            Context.text_entry = this;
            cursor = text.size();
            redrawText();
        }
        void stopTextEntry(){
            if (Context.text_entry== this){
                SDL_StopTextInput();
                Context.text_entry = nullptr;
            }
        }
        void onLeftClick(int clicks) override {
            if (clicks == 2){
                beginTextEntry();
            }
        }

        virtual void onReturn(std::string /*input_text*/){
            setText("");
            stopTextEntry();
        }

        void redrawText(){
            Context.text_renderer.writeToTexture(text, &image, autosize_x ? -1 : rect.w, autosize_y ? -1 : rect.h, &cursor_rect, cursor);
            cursor_rect += rect.pos();
            rect.w = image.width();
            rect.h = image.height();
        }

        void render(Pos offset) override {
            image.render(rect.pos() + offset);

            if (Context.text_entry == this){
                drawColor(ColorHSVA(0.1, 1, 0.9, 0.6 * reRange(sine(GUI::getProgramTime() * 3))));
                drawRect(cursor_rect + offset);
            }
        }

        private:
        Image image;
        std::string text;
        size_t cursor = 0;
        Rect cursor_rect;
    };

    struct GUIRootWindow : Window {
        ~GUIRootWindow(){
            if (Context.current_window == this){
                Context.current_window = nullptr;
            }
            Context.quit = true;
        }
        void render(Pos /*offset*/) override {
            drawColor(0xFF404040);
            drawRect(rect);
            renderElements(Pos(0, 0));
        }
    } rootwindow;

    Window::~Window(){
        clear();
        if (Context.dragging_window == this){
            Context.dragging_window = nullptr;
        }
        if (Context.current_window == this && this != &rootwindow){
            Context.focusTo(parent);
        }
    }

    void addKeyboardCommand(SDL_Scancode trigger_key, void (*handler)(const Uint8*)){
        auto pair = std::pair<SDL_Scancode, std::vector<SDL_Scancode>>(trigger_key, {});
        auto it = Context.commands.find(pair);
        if (it == Context.commands.end()){
            Context.commands[pair] = handler;
        } else {
            throw;
        }
    }
    void addKeyboardCommand(SDL_Scancode trigger_key, std::vector<SDL_Scancode> required_keys, void (*handler)(const Uint8*)){
        auto pair = std::pair<SDL_Scancode, std::vector<SDL_Scancode>>(trigger_key, required_keys);
        auto it = Context.commands.find(pair);
        if (it == Context.commands.end()){
            Context.commands[pair] = handler;
        } else {
            throw;
        }
    }
    void setQuitHandler(bool (*handler)()){
        Context.quit_handler = handler;
    }

    double getProgramTime(){
        return Context.program_time;
    }

    Pos getScreenSize(){
        return Context.screensize;
    }

    void init(Pos size = Pos(500, 500), std::string name = "Behold"){//TODO: this might goof up, make bool
        rootwindow.rect.w = size.x;
        rootwindow.rect.h = size.y;

        Context.current_window = &rootwindow;

        Context.screensize = size;

        SDL_Init(SDL_INIT_EVERYTHING);
        IMG_Init(IMG_INIT_PNG);

        window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, Context.screensize.x, Context.screensize.y, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        srand(unsigned(time(nullptr)));

        Context.ticks = SDL_GetTicks();

        // Context.text_renderer = TextRenderer(); // LOL nice
        Context.text_renderer.loadFont("font.png", 7, 12, 28, 2);
    }

    void run(){
        Context.quit = false;
        SDL_Event _event;
        while (!Context.quit){
            while(SDL_PollEvent(&_event)){
                switch (_event.type){
                    case SDL_QUIT:
                        if (Context.quit_handler){
                            Context.quit = Context.quit_handler();
                        } else {
                            Context.quit = true;
                        }
                    break;
                    case SDL_WINDOWEVENT:
                        if (_event.window.event == SDL_WINDOWEVENT_RESIZED){
                            Context.screensize.x = _event.window.data1;
                            Context.screensize.y = _event.window.data2;
                        }
                    break;
                    case SDL_MOUSEBUTTONDOWN:
                        if (SDL_IsTextInputActive()){
                            SDL_StopTextInput();
                            Context.text_entry = nullptr;
                        }
                        if (_event.button.button == SDL_BUTTON_LEFT){
                            if (Window* hit_window = rootwindow.windowAt(Context.mouse_pos)){
                                Context.focusTo(hit_window);
                                hit_window->onLeftClick(_event.button.clicks);
                            }
                        }
                        if (_event.button.button == SDL_BUTTON_RIGHT){
                            if (Window* hit_window = rootwindow.windowAt(Context.mouse_pos)){
                                Context.focusTo(hit_window);
                                hit_window->onRightClick(_event.button.clicks);
                            }
                        }
                    break;
                    case SDL_MOUSEBUTTONUP:
                        if (_event.button.button == SDL_BUTTON_LEFT){
                            if (Context.current_window){
                                Context.current_window->onLeftRelease();
                            }
                            if (Context.dragging_window){
                                Window* hover_window = rootwindow.windowAt(Context.mouse_pos);
                                hover_window->onreleaseElement(Context.dragging_window);
                                Context.dragging_window = nullptr;
                            }
                        }
                        if (_event.button.button == SDL_BUTTON_RIGHT){
                            if (Context.current_window){
                                Context.current_window->onRightRelease();
                            }
                            if (Context.dragging_window){
                                Window* hover_window = rootwindow.windowAt(Context.mouse_pos);
                                hover_window->onreleaseElement(Context.dragging_window);
                                Context.dragging_window = nullptr;
                            }
                        }
                    break;
                    case SDL_MOUSEMOTION:
                        Context.mouse_pos.x = _event.button.x;
                        Context.mouse_pos.y = _event.button.y;
                    break;
                    case SDL_KEYDOWN:
                        if (Context.text_entry){
                            switch (_event.key.keysym.scancode){
                                case SDL_SCANCODE_BACKSPACE:
                                    Context.text_entry->textEditBackspace();
                                    break;
                                case SDL_SCANCODE_DELETE:
                                    Context.text_entry->textEditDelete();
                                    break;
                                case SDL_SCANCODE_LEFT:
                                    Context.text_entry->textEditLeft();
                                    break;
                                case SDL_SCANCODE_RIGHT:
                                    Context.text_entry->textEditRight();
                                    break;
                                case SDL_SCANCODE_HOME:
                                    Context.text_entry->textEditHome();
                                    break;
                                case SDL_SCANCODE_END:
                                    Context.text_entry->textEditEnd();
                                    break;
                                case SDL_SCANCODE_RETURN:
                                    Context.text_entry->textEditReturn();
                                    break;
                                case SDL_SCANCODE_ESCAPE:
                                    Context.text_entry->stopTextEntry();
                                    break;
                                default:
                                    break;
                            }
                        } else {
                            
                            auto it = Context.commands.begin();
                            size_t max = 0;
                            auto current_it = Context.commands.end();
                            while (it != Context.commands.end()){
                                if (it->first.first == _event.key.keysym.scancode){
                                    bool match = true;
                                    for (size_t i = 0; i < it->first.second.size() && match; i++){
                                        match = Context.keystates[it->first.second[i]];
                                    }
                                if (match && it->first.second.size() >= max){
                                        max = it->first.second.size();
                                        current_it = it;
                                    }
                                }
                                it++;
                            }

                            if (current_it != Context.commands.end()){
                                current_it->second(Context.keystates);
                            } else if (Context.current_window){
                                Context.current_window->onKeyDown(_event.key.keysym.scancode);
                            }
                        }
                    break;
                    case SDL_KEYUP:
                        if (Context.current_window){
                            Context.current_window->onKeyUp(_event.key.keysym.scancode);
                        }
                    break;
                    case SDL_TEXTINPUT:
                        if (Context.text_entry){
                            Context.text_entry->type(_event.text.text);
                        }
                    break;
                }
            }

            if (Context.dragging_window){
                Context.dragging_window->rect.x = Context.mouse_pos.x - Context.drag_offset.x;
                Context.dragging_window->rect.y = Context.mouse_pos.y - Context.drag_offset.y;
                Context.dragging_window->onDrag();
            }

            Window *hover_window = rootwindow.windowAt(Context.mouse_pos);
            if (hover_window){
                hover_window->onHover();
            }

            Context.ticks = SDL_GetTicks();
            Context.program_time = Context.ticks / 1000.0;

            drawColor(0xFF000000);
            clearScreen();
            rootwindow.rect = Rect(Context.screensize);
            rootwindow.render(Pos(0, 0));

            if (Context.keystates[SDL_SCANCODE_LALT] || Context.keystates[SDL_SCANCODE_RALT]){
                Rect rect = Rect(Context.current_window->rootPos(), Context.current_window->rect.size());
                drawColor(0x80FFFF00);
                drawRectBorder(rect, false);
                drawRectBorder(rect, true);
            }

            updateScreen();

            SDL_Delay(20);
        }

        rootwindow.clear();

        SDL_Quit();
    }
}