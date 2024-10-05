//
// Created by Rafael Reip on 04.10.24.
//

#ifndef RENDERER_H
#define RENDERER_H

#include "lib/util/collection/ArrayList.h"
#include "lib/util/graphic/BufferedLinearFrameBuffer.h"
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/async/Runnable.h"
#include "lib/util/io/file/File.h"
#include "lib/util/async/Thread.h"
#include "lib/util/graphic/Ansi.h"
#include "lib/util/base/Address.h"
#include "lib/util/time/Timestamp.h"
#include "lib/libc/stdlib.h"

#include "Layer.h"
#include "Pic.h"
#include "DataWrapper.h"

class Renderer : public Util::Async::Runnable {
public:
    explicit Renderer(RenderData *renderData, Util::Graphic::LinearFrameBuffer *lfb);

    ~Renderer() override = default;

    void run() override;

    void prepareBase();

    static uint32_t *newBuffer(int size);

    void renderMouse();

    void removeMouse();

    void renderResult();

    void renderWorkArea();

    void renderOverlay();

    void renderLayers();

    static uint32_t blendPixels(uint32_t lower, uint32_t upper);

    static void blendBuffers(uint32_t *lower, const uint32_t *upper, int size);

    static void blendBuffers(uint32_t *lower, const uint32_t *upper, int lx, int ly, int ux, int uy, int px, int py);


private:
    int screenX, screenY, pitch;
    int workAreaX, workAreaY;
    Util::Graphic::LinearFrameBuffer *lfb;
    RenderData *rData;
    uint32_t *buff_lfb;
    uint32_t *buff_result;
    uint32_t *buff_base, *buff_workarea, *buff_gui;
    uint32_t *buff_overlay, *buff_layers;
    uint32_t *buff_under_current, *buff_over_current;
    int lastMouseX, lastMouseY;
};

#endif // RENDERER_H