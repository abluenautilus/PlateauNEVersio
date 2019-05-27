#ifndef DSJ_VALLEY_WIDGETS_HPP
#define DSJ_VALLEY_WIDGETS_HPP

#include "Valley.hpp"
#include "window.hpp"
#include <functional>

// Dynamic Panel

struct PanelBorderWidget : TransparentWidget {
	void draw(NVGcontext *vg) override;
};

struct DynamicPanelWidget : FramebufferWidget {
    int* mode;
    int oldMode;
    std::vector<std::shared_ptr<SVG>> panels;
    SVGWidget* visiblePanel;
    PanelBorderWidget* border;

    DynamicPanelWidget();
    void addPanel(std::shared_ptr<SVG> svg);
    void step() override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// View mode

enum DynamicViewMode {
    ACTIVE_HIGH_VIEW,
    ACTIVE_LOW_VIEW,
    ALWAYS_ACTIVE_VIEW
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Switch

struct DynamicSwitchWidget : Switch {
    std::vector<std::shared_ptr<SVG>> frames;
    FramebufferWidget *fb;
    SVGWidget* sw;
    int* visibility;
    DynamicViewMode viewMode;

    DynamicSwitchWidget();
    void addFrame(std::shared_ptr<SVG> svg);
    void step() override;
    void onChange(const event::Change &e) override;
};

template <class TDynamicSwitch>
DynamicSwitchWidget* createDynamicSwitchWidget(Vec pos, Module *module, int paramId,
                                               float minValue, float maxValue, float defaultValue,
                                               int* visibilityHandle, DynamicViewMode viewMode) {
	DynamicSwitchWidget *dynSwitch = new TDynamicSwitch();
	dynSwitch->box.pos = pos;
	//dynSwitch->module = module;
	dynSwitch->paramQuantity->paramId = paramId;
	//dynSwitch->setLimits(minValue, maxValue);
	//dynSwitch->setDefaultValue(defaultValue);
    if (module) {
		module->configParam(paramId, minValue, maxValue, defaultValue);
	}
    dynSwitch->visibility = visibilityHandle;
    dynSwitch->viewMode = viewMode;
	return dynSwitch;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic lights

struct DynamicModuleLightWidget : MultiLightWidget {
	Module *module = NULL;
	int firstLightId;
    int* visibility = nullptr;
    DynamicViewMode viewMode = ACTIVE_HIGH_VIEW;

	void step() override;
};

template<class TDynamicModuleLightWidget>
DynamicModuleLightWidget *createDynamicLight(Vec pos, Module *module, int firstLightId,
                                             int* visibilityHandle, DynamicViewMode viewMode) {
	DynamicModuleLightWidget *light = new TDynamicModuleLightWidget();
	light->box.pos = pos;
	light->module = module;
	light->firstLightId = firstLightId;
    light->visibility = visibilityHandle;
    light->viewMode = viewMode;
	return light;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic knob

enum DynamicKnobMotion {
    SMOOTH_MOTION,
    SNAP_MOTION
};

struct DynamicKnob : virtual Knob {
	/** Angles in radians */
	float minAngle, maxAngle;
	/** Not owned */
    FramebufferWidget *fb;
	TransformWidget *tw;
	SVGWidget *sw;
    CircularShadow *shadow;
    int* _visibility;
    DynamicViewMode _viewMode;

	DynamicKnob();
	void setSVG(std::shared_ptr<SVG> svg);
	void step() override;
	void onChange(const event::Change &e) override;
};

template <class TDynamicKnob>
DynamicKnob* createDynamicKnob(const Vec& pos,
                               Module* module,
                               int paramId,
                               int* visibilityHandle,
                               DynamicViewMode viewMode,
                               float minValue,
                               float maxValue,
                               float defaultValue,
                               DynamicKnobMotion motion) {
    DynamicKnob* knob = new TDynamicKnob;
    //knob->module = module;
    knob->box.pos = pos;
    knob->paramQuantity->paramId = paramId;
    //knob->setLimits(minValue, maxValue);
    //knob->setDefaultValue(defaultValue);
    if (module) {
		module->configParam(paramId, minValue, maxValue, defaultValue);
	}
    knob->_visibility = visibilityHandle;
    knob->_viewMode = viewMode;
    if(motion == SNAP_MOTION) {
        knob->snap = true;
    }
    return knob;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SVGStepSlider : app::SvgSlider {
	void onChange(const event::Change& e) override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic text

struct DynamicText : TransparentWidget {
    std::shared_ptr<std::string> text;
    std::shared_ptr<Font> font;
    int size;
    float blur;
    int* visibility;
    DynamicViewMode viewMode;

    enum ColorMode {
        COLOR_MODE_WHITE = 0,
        COLOR_MODE_BLACK,
        COLOR_MODE_RED
    };

    enum FontMode {
        FONT_MODE_ALTEDIN = 0,
        FONT_MODE_7SEG
    };

    int* colorHandle;
    NVGcolor textColor;
    NVGcolor customColor;
    NVGalign horzAlignment;
    NVGalign vertAlignment;

    DynamicText();
    virtual void draw(NVGcontext* vg) override;
    void step() override;
    void setFont(const FontMode& newFontMode);
};

DynamicText* createDynamicText(const Vec& pos, int size, std::string text,
                               int* visibilityHandle, DynamicViewMode viewMode);
DynamicText* createDynamicText(const Vec& pos, int size, std::string text,
                               int* visibilityHandle, int* colorHandle, DynamicViewMode viewMode);
DynamicText* createDynamicText(const Vec& pos, int size, std::shared_ptr<std::string> text,
                               int* visibilityHandle, DynamicViewMode viewMode);
DynamicText* createDynamicText(const Vec& pos, int size, std::shared_ptr<std::string> text,
                               int* visibilityHandle, int* colorHandle, DynamicViewMode viewMode);

struct DynamicFrameText : DynamicText {
    int* itemHandle;
    std::vector<std::string> textItem;

    DynamicFrameText();
    void addItem(const std::string& item);
    void draw(NVGcontext* vg) override;
};

template<class T>
class DynamicValueText : public TransformWidget {
public:
    std::shared_ptr<Font> font;
    int size;
    int* visibility;
    DynamicViewMode viewMode;

    enum ColorMode {
        COLOR_MODE_WHITE = 0,
        COLOR_MODE_BLACK
    };
    int* colorHandle;
    NVGcolor textColor;

    DynamicValueText(std::shared_ptr<T> value, std::function<std::string(T)> valueToText)  {
        font = Font::load(assetPlugin(pluginInstance, "res/din1451alt.ttf"));
        size = 16;
        visibility = nullptr;
        colorHandle = nullptr;
        viewMode = ACTIVE_HIGH_VIEW;
        _value = value;
        _valueToText = valueToText;
    }

    void draw(NVGcontext* vg) override {
        nvgFontSize(vg, size);
        nvgFontFaceId(vg, font->handle);
        nvgTextLetterSpacing(vg, 0.f);
        Vec textPos = Vec(0.f, 0.f);
        if(colorHandle != nullptr) {
            switch((ColorMode)*colorHandle) {
                case COLOR_MODE_WHITE: textColor = nvgRGB(0xFF,0xFF,0xFF); break;
                case COLOR_MODE_BLACK: textColor = nvgRGB(0x14,0x14, 0x14); break;
                default: textColor = nvgRGB(0xFF,0xFF,0xFF);
            }
        }
        else {
            textColor = nvgRGB(0xFF,0xFF,0xFF);
        }

        nvgFillColor(vg, textColor);
        nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
        nvgText(vg, textPos.x, textPos.y, _text.c_str(), NULL);
    }

    void step() override {
        _text = _valueToText(*_value);
        if(visibility != nullptr) {
            if(*visibility) {
                visible = true;
            }
            else {
                visible = false;
            }
            if(viewMode == ACTIVE_LOW_VIEW) {
                visible = !visible;
            }
        }
    }
private:
    std::shared_ptr<T> _value;
    std::function<std::string(T)> _valueToText;
    std::string _text;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Dynamic Choices

struct DynamicItem : MenuItem {
    unsigned long _itemNumber;
    unsigned long* _choice;
    DynamicItem(unsigned long itemNumber);
    void onAction(const event::Action &e) override;
};

struct DynamicChoice : ChoiceButton {
    unsigned long* _choice;
    std::vector<std::string> _items;
    std::shared_ptr<std::string> _text;
    std::shared_ptr<Font> _font;
    int* _visibility;
    int _textSize;
    DynamicViewMode _viewMode;
    DynamicChoice();
    void step() override;
    void onAction(const event::Action &e) override;
    void draw(NVGcontext* vg) override;
};

DynamicChoice* createDynamicChoice(const Vec& pos,
                                   float width,
                                   const std::vector<std::string>& items,
                                   unsigned long* choiceHandle,
                                   int* visibilityHandle,
                                   DynamicViewMode viewMode);

template<class T = SVGKnob>
T *createValleyKnob(Vec pos, Module *module, int paramId, float minValue, float maxValue,
                    float defaultValue, float minAngle, float maxAngle, DynamicKnobMotion motion) {
    /*T *o = Component::create<T>(pos, module);
	o->paramId = paramId;
	o->setLimits(minValue, maxValue);
	o->setDefaultValue(defaultValue);
    o->minAngle = minAngle;
    o->maxAngle = maxAngle;
    if(motion == SNAP_MOTION) {
        o->snap = true;
    }
	return o;*/
    T *o = createParam<T>(pos, module, paramId);
	//o->setLimits(minValue, maxValue);
    if (module) {
		module->configParam(paramId, minValue, maxValue, defaultValue);
	}
    o->minAngle = minAngle;
    o->maxAngle = maxAngle;
    if(motion == SNAP_MOTION) {
        o->snap = true;
    }
	return o;
}

#endif
