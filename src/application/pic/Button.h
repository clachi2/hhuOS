//
// Created by Rafael Reip on 06.10.24.
//

#ifndef HHUOS_BUTTON_H
#define HHUOS_BUTTON_H

#include <cstdint>
#include "lib/util/graphic/Color.h"

#include "helper.h"

namespace Util::Graphic {
    class LinearFrameBuffer;

    class PixelDrawer;

    class StringDrawer;

    class LineDrawer;
}

class DataWrapper;

enum ButtonType {
    NONE,
    METHOD,
    INT_VALUE,
    DOUBLE_VALUE,
    BOOLEAN,
    CONFIRM,
    LAYER,
    INPUT
};

class Button {
public:

    explicit Button(DataWrapper *data);

    ~Button();

    void processClick(int relX, int relY);

    void showClick(int relX, int relY);

    void showHover(int relX, int relY);

    void removeInteraction();

    uint32_t *getBuffer();

    Button *setInfo(const char *string);

    Button *setHotkey(char hotkey);

    Button *set16Bitmap(uint8_t *bitmap);

    Button *setMethodButton(void (*method)(DataWrapper *));

    Button *setIntValueButton(int *value);

    Button *setIntValueButton(int *value, int limit_low, int limit_high);

    Button *setDoubleValueButton(double *dvalue);

    Button *setDoubleValueButton(double *dvalue, double limit_low, double limit_high);

    Button *setBooleanButton(bool *bvalue);

    Button *setConfirmButton(void (*cancel)(DataWrapper *), void (*ok)(DataWrapper *));

    Button *setLayerButton(int layerNum);

    Button *setInputButton(Util::String *input, bool *captureInput);

    Button *setRenderFlagMethod(void (RenderFlags::*rFlagMethod)());

    Button *setSecondRenderFlagMethod(void (RenderFlags::*rFlagMethod)());

    Button *changeGreenIfTool(Tool tool);

    Button *changeGreenIfShape(Shape shape);

    Button *setColor(int *colorA, int *colorR, int *colorG, int *colorB);

    Button *setAppearTopOnChange(bool set);

    Button *setAppearBottomOnChange(bool set);

    bool bufferChanged;

    void render();

private:

    void renderBorder(uint32_t color);

    void renderBackground(int x1, int x2, uint32_t color);

    void renderMethod();

    void renderValue(const char *text);

    void renderIntValue();

    void renderDoubleValue();

    void renderBoolean();

    void renderConfirm();

    void renderLayer();

    void renderInput();

    DataWrapper *data;
    uint32_t *buffer;
    Util::Graphic::LinearFrameBuffer *lfb;
    Util::Graphic::PixelDrawer *pixelDrawer;
    Util::Graphic::LineDrawer *lineDrawer;
    Util::Graphic::StringDrawer *stringDrawer;
    Util::Graphic::Color cblack, cgray, cgreen, cdarkgray, cred;
    uint32_t black, gray, green, darkgray, red;
    ButtonType type;
    bool click;
    bool hover;
    int mouseX, mouseY;

    // optionals
    const char *info;
    char hotkey;
    uint8_t *bitmap;
    int *intValue = nullptr;
    int intLimitLow = 0, intLimitHigh = 0;
    bool hasIntLimits = false;
    double *doubleValue = nullptr;
    double doubleLimitLow = 0, doubleLimitHigh = 0;
    bool hasDoubleLimits = false;
    bool *boolValue = nullptr;
    Util::String *input = nullptr;
    bool *captureInput = nullptr;
    Tool setGreenTool = Tool::NOTHING;
    Shape setGreenShape = Shape::DEFAULT;
    bool showcolor = false;
    int *colorA = nullptr, *colorR = nullptr, *colorG = nullptr, *colorB = nullptr;
    bool appearTopOnChange = false, appearBottomOnChange = false;
    int layerNum = -1;

    void (*method1)(DataWrapper *data) = nullptr;

    void (*method2)(DataWrapper *data) = nullptr;

    void (RenderFlags::*rFlagMethod)() = nullptr;

    void (RenderFlags::*rFlagMethod2)() = nullptr;

};

#endif // HHUOS_BUTTON_H
