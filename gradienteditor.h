#pragma once

#include <gradient.h>
#include <gui.h>

struct GradientEditor : GUI::Window {
	GradientEditor(Gradient& _gradient) : gradient(_gradient){
		rect = Rect(350, 50);
		image.resize(350, 50);
		addElement(Pos(360, 0), random_gradient_btn = new RandomGradientBtn(*this));
		addElement(Pos(360, 30), reset_btn = new ResetBtn(*this));
		addElement(Pos(0, 150), color_editor = new ColorEditor(this));
		refreshHandles();
		redrawGradient();
	}

	void onLeftClick(int clicks) override {
		double pos = localMousePos().x / (double)rect.w;
		addNode(gradient.getColorAt(pos), pos);
	}

	void addNode(Color color, double pos){
		Gradient::ColorNode* node = gradient.addNode(color, pos);
		NodeHandle* handle = new NodeHandle(node, *this);
		addElement(handle);
		color_editor->switchTo(handle);
	}

	void render(Pos offset) override {

		int width = GUI::getScreenSize().x;
		if (width != prev_width){
			rect.w = width - 270 - random_gradient_btn->rect.w - 20;
			random_gradient_btn->rect.x = rect.w + 10;
			reset_btn->rect.x = rect.w + 10;
			refreshHandles();
			redrawGradient();
			prev_width = width;
		}

		image.render(rect.pos() + offset);
		drawColor(0xFF000000);
		drawRectBorder(rect + offset);
		renderElements(offset);
	}

	void redrawGradient(){
		if (image.width() != rect.w || image.height() != rect.h){
			image.resize(rect.w, rect.h);
		}
		for (int x = 0; x < rect.w; x++){
			Uint32 color = gradient.getColorAt(x / (double)rect.w);
			for (int y = 0; y < rect.h; y++){
				image.setPixel(x, y, color);
			}
		}
	}

	void randomize(int num_colors){
		gradient.randomize(num_colors);
		refreshHandles();
		redrawGradient();
	}

	void refreshHandles(){
		std::vector<Element*> temp = elements;
		for (int i = 0; i < temp.size(); i++){
			if (temp[i] != (Element*)random_gradient_btn && temp[i] != (Element*)reset_btn && temp[i] != (Element*)color_editor){
				temp[i]->close();
			}
		}
		for (int i = 0; i < gradient.nodes.size(); i++){
			NodeHandle* handle = new NodeHandle(gradient.nodes[i], *this);
			addElement(handle);
		}
	}

	Gradient& gradient;
	Image image;

	struct NodeHandle : GUI::Window {
		Gradient::ColorNode* node = nullptr;
		GradientEditor& editor;
		int old_x;
		NodeHandle(Gradient::ColorNode* _node, GradientEditor& _editor) : editor(_editor){
			node = _node;
			rect.w = 10;
			rect.h = 70;
			rect.y = 0;
		}
		~NodeHandle(){
			if (editor.color_editor->node_handle == this){
				editor.color_editor->hide();
			}
		}
		void render(Pos offset) override {
			rect.x = node->pos * editor.rect.w - rect.w / 2;
			drawColor(0xFFFFFF ^ node->color.toInt() | 0xFF000000);
			drawLine(rect.pos() + offset + Pos(rect.w / 2, 0), rect.pos() + offset + Pos(rect.w / 2, rect.h));
			drawColor(0xFF000000);
			drawRectBorder(Rect(0, rect.h - rect.w, rect.w, rect.w) + offset + rect.pos(), true);
			drawColor(node->color);
			drawRect(Rect(0, rect.h - rect.w, rect.w, rect.w) + offset + rect.pos());
		}
		void onLeftClick(int clicks) override {
			if (clicks == 2){
				editor.gradient.removeNode(node);
				editor.redrawGradient();
				g_fractal().rerender = true;
				close();
			} else {
				editor.color_editor->switchTo(this);

				old_x = rect.x;
				startDrag();
			}
		}
		void onDrag() override {
			rect.x = std::min(std::max(rect.x, -rect.w / 2), editor.rect.w - rect.w / 2);
			rect.y = 0;
			if (old_x != rect.x){
				node->pos = (rect.x + rect.w / 2) / (double)editor.rect.w;
				editor.redrawGradient();
				g_fractal().rerender = true;
			}
			old_x = rect.x;
		}
	};

	struct ColorEditor : GUI::Window {
		ColorEditor(GradientEditor* _gradient_editor){
			gradient_editor = _gradient_editor;
			node_handle = nullptr;
			visible = false;
			addElement(Pos(10, 10), red_slider = new RedSlider(this, &red, &update_rgb));
			addElement(Pos(10, 40), green_slider = new GreenSlider(this, &green, &update_rgb));
			addElement(Pos(10, 70), blue_slider = new BlueSlider(this, &blue, &update_rgb));
			addElement(Pos(10, 100), hue_slider = new HueSlider(this, &hue, &update_hsv));
			addElement(Pos(10, 130), saturation_slider = new SaturationSlider(this, &saturation, &update_hsv));
			addElement(Pos(10, 160), value_slider = new ValueSlider(this, &value, &update_hsv));
			red = 0;
			green = 0;
			blue = 0;
			hue = 0;
			saturation = 0;
			value = 0;
			rect = Rect(red_slider->rect.w + 20, 10 + (6 * (red_slider->rect.h + 10)));
		}

		void hide(){
			node_handle = nullptr;
			visible = false;
		}
		void switchTo(NodeHandle* handle){
			visible = true;
			node_handle = handle;
			updateSliders();
		}

		void updateSliders(){
			Color color = node_handle->node->color;
			hue = color.getH();
			saturation = color.getS();
			value = color.getV();
			red = color.getR();
			green = color.getG();
			blue = color.getB();
			red_slider->grabber->rect.x = (red_slider->rect.w - red_slider->grabber->rect.w) * color.getR();
			green_slider->grabber->rect.x = (green_slider->rect.w - green_slider->grabber->rect.w) * color.getG();
			blue_slider->grabber->rect.x = (blue_slider->rect.w - blue_slider->grabber->rect.w) * color.getB();
			hue_slider->grabber->rect.x = (hue_slider->rect.w - hue_slider->grabber->rect.w) * color.getH();
			saturation_slider->grabber->rect.x = (saturation_slider->rect.w - saturation_slider->grabber->rect.w) * color.getS();
			value_slider->grabber->rect.x = (value_slider->rect.w - value_slider->grabber->rect.w) * color.getV();
		}

		struct Slider : GUI::Window {
			Slider(ColorEditor* _color_editor, double* _target, bool* _flag){
				color_editor = _color_editor;
				target = _target;
				flag = _flag;
				rect = Rect(255, 20);
				addElement(grabber = new Grabber(this));
			}

			void onLeftClick(int clicks) override {
				grabber->rect.x = localMousePos().x - (grabber->rect.w / 2);
				grabber->startDrag();
			}

			struct Grabber : GUI::Window {
				Grabber(Slider* _slider){
					slider = _slider;
					rect = Rect(20, 20);
				}
				void onLeftClick(int clicks) override {
					startDrag();
				}
				void onDrag() override {
					rect.y = 0;
					if (rect.x < 0){
						rect.x = 0;
					}
					if (rect.x > slider->rect.w - rect.w){
						rect.x = slider->rect.w - rect.w;
					}
					*(slider->target) = rect.x / (double)(slider->rect.w - rect.w);
					*slider->flag = true;
				}
				void render(Pos offset) override {
					drawColor(0xFF000000);
					drawRect(rect + offset);
				}
				Slider* slider;
			} *grabber;


			bool* flag;
			double* target;
			ColorEditor* color_editor;
		};

		struct RedSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				for (int x = 0; x < rect.w; x++){
					Color color = color_editor->node_handle->node->color;
					drawColor(ColorRGBA(x / (double)rect.w, color.getG(), color.getB()));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		struct GreenSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				Color color = color_editor->node_handle->node->color;
				for (int x = 0; x < rect.w; x++){
					drawColor(ColorRGBA(color.getR(), x / (double)rect.w, color.getB()));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		struct BlueSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				Color color = color_editor->node_handle->node->color;
				for (int x = 0; x < rect.w; x++){
					drawColor(ColorRGBA(color.getR(), color.getG(), x / (double)rect.w));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		struct HueSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				Color color = color_editor->node_handle->node->color;
				for (int x = 0; x < rect.w; x++){
					drawColor(ColorHSVA(x / (double)rect.w, color.getS(), color.getV()));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		struct SaturationSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				for (int x = 0; x < rect.w; x++){
					Color color = color_editor->node_handle->node->color;
					drawColor(ColorHSVA(color.getH(), x / (double)rect.w, color.getV()));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		struct ValueSlider : Slider {
			using Slider::Slider;
			void render(Pos offset) override {
				for (int x = 0; x < rect.w; x++){
					Color color = color_editor->node_handle->node->color;
					drawColor(ColorHSVA(color.getH(), color.getS(), x / (double)rect.w));
					drawLine(rect.pos() + offset + Pos(x, 0), rect.pos() + offset + Pos(x, rect.h));
				}
				drawColor(0xFF000000);
				drawRectBorder(rect + offset, true);
				renderElements(offset);
			}
		};

		void render(Pos offset) override {
			if (node_handle){
				rect.x = std::max(0, std::min(node_handle->rootPos().x - gradient_editor->rootPos().x - rect.w / 2, gradient_editor->rect.w - rect.w));
				if (update_rgb || update_hsv){
					if (update_rgb){
						Color color = ColorRGBA(red, green, blue);
						node_handle->node->color = color;
					} else {
						Color color = ColorHSVA(hue, saturation, value);
						node_handle->node->color = color;
					}
					updateSliders();
					gradient_editor->redrawGradient();
					g_fractal().abortRender();
					g_fractal().beginRender();
					update_rgb = false;
					update_hsv = false;
				}

				drawColor(0xFF000000);
				drawLine(node_handle->rootPos() + Pos(0, node_handle->rect.h), rootPos());
				drawLine(node_handle->rootPos() + Pos(node_handle->rect.w, node_handle->rect.h), rootPos() + Pos(rect.w, 0));
			}
			drawColor(node_handle->node->color);
			drawRect(rect + offset);
			drawColor(0xFF000000);
			drawRectBorder(rect + offset);
			renderElements(offset);
		}

		double red, green, blue, hue, saturation, value;
		bool update_rgb = false;
		bool update_hsv = false;
		RedSlider* red_slider;
		GreenSlider* green_slider;
		BlueSlider* blue_slider;
		HueSlider* hue_slider;
		SaturationSlider* saturation_slider;
		ValueSlider* value_slider;
		NodeHandle* node_handle;
		GradientEditor* gradient_editor;

	} *color_editor = nullptr;

	struct RandomGradientBtn : GUI::Window {
		GradientEditor& editor;
		RandomGradientBtn(GradientEditor& _editor) : editor(_editor){
			rect = Rect(120, 20);
			addElement(Pos(10, 0), new GUI::Text("Random Gradient"));
		}
		void onLeftClick(int clicks) override {
			g_fractal().params.gradient.randomize(3 + rand() % 28);
			g_fractal().rerender = true;
			editor.refreshHandles();
			editor.redrawGradient();
		}
		void render(Pos offset) override {
			drawColor(0xFFFFFF00);
			drawRect(rect + offset);
			renderElements(offset);
		}
	}* random_gradient_btn;

	struct ResetBtn : GUI::Window {
		GradientEditor& editor;
		ResetBtn(GradientEditor& _editor) : editor(_editor){
			rect = Rect(120, 20);
			addElement(Pos(10, 0), new GUI::Text("Reset"));
		}
		void onLeftClick(int clicks) override {
			g_fractal().params.gradient.clear();
			g_fractal().params.gradient.addNode(0xFFFFFFFF, 0.0);
			g_fractal().params.gradient.addNode(0xFF000000, 1.0);
			g_fractal().rerender = true;
			editor.refreshHandles();
			editor.redrawGradient();
		}
		void render(Pos offset) override {
			drawColor(0xFFA0A0A0);
			drawRect(rect + offset);
			renderElements(offset);
		}
	}* reset_btn;
	int prev_width;
};