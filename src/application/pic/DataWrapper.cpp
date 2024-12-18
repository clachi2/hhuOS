//
// Created by Rafael Reip on 04.10.24.
//

#include "DataWrapper.h"

#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/graphic/BufferedLinearFrameBuffer.h"
#include "lib/util/io/stream/FileInputStream.h"
#include "lib/util/io/key/KeyDecoder.h"
#include "lib/util/io/key/layout/DeLayout.h"
#include "lib/util/graphic/Ansi.h"

#include "MessageHandler.h"
#include "Settings.h"
#include "History.h"
#include "Button.h"
#include "GuiLayer.h"
#include "Layers.h"

/**
 * Constructor for DataWrapper.
 * Initializes screen, input, rendering, layers, GUI, settings, and work variables.
 *
 * @param lfbFile Pointer to the file used for the LinearFrameBuffer.
 */
DataWrapper::DataWrapper(Util::Io::File *lfbFile) {
    // screen
    lfb = new Util::Graphic::LinearFrameBuffer(*lfbFile);
    blfb = new Util::Graphic::BufferedLinearFrameBuffer(*lfb);
    screenX = lfb->getResolutionX(), screenY = lfb->getResolutionY(), pitch = lfb->getPitch(), screenAll = screenX * screenY;
    workAreaX = screenX - 200, workAreaY = screenY, workAreaAll = workAreaX * workAreaY;
    guiX = 200, guiY = screenY, guiAll = guiX * guiY;
    buttonCount = screenY / 30;
    Util::Graphic::Ansi::prepareGraphicalApplication(true);

    // input
    auto mouseFile = Util::Io::File("/device/mouse");
    mouseInputStream = new Util::Io::FileInputStream(mouseFile);
    mouseInputStream->setAccessMode(Util::Io::File::NON_BLOCKING);
    Util::Io::File::setAccessMode(Util::Io::STANDARD_INPUT, Util::Io::File::NON_BLOCKING);
    keyDecoder = new Util::Io::KeyDecoder(new Util::Io::DeLayout());
    xMovement = 0, yMovement = 0;
    mouseX = 0, mouseY = 0;
    leftButtonPressed = false, oldLeftButtonPressed = false, newlyPressed = false;
    mouseClicks = new Util::ArrayBlockingQueue<Util::Pair<int, int>>(500); // should be more than enough :D
    clickStartedOnGui = true;
    lastInteractedButton = -1;
    currentInput = new Util::String();
    captureInput = false;
    lastScancode = 0;

    // rendering
    flags = new RenderFlags();
    mHandler = new MessageHandler(workAreaX, workAreaY);
    mHandler->setPrintBool(true);

    // layers
    history = new History(mHandler);
    layers = new Layers(mHandler, history);

    // gui
    guiLayers = new Util::HashMap<Util::String, GuiLayer *>();
    currentGuiLayer = nullptr;
    currentGuiLayerBottom = nullptr;
    textButton = nullptr;
    inMainMenu = true;

    // settings
    settings = new Settings(mHandler);

    // work vars
    running = true;
    currentTool = NOTHING;
    moveX = -1;
    moveY = -1;
    rotateDeg = -1;
    scale = -1.0;
    toolCorner = BOTTOM_RIGHT;
    cropLeft = -1;
    cropRight = -1;
    cropTop = -1;
    cropBottom = -1;
    penSize = -1;
    colorA = 128, colorR = 0, colorG = 255, colorB = 0;
    combineFirst = 0;
    combineSecond = 1;
    layerX = 0, layerY = 0, layerW = workAreaX, layerH = workAreaY;
    dupeIndex = 0;
    shapeX = -1, shapeY = -1, shapeW = -1, shapeH = -1;
    currentShape = RECTANGLE;
    replaceColorX = -1, replaceColorY = -1;
    replaceColorTolerance = 0.0;
}

/**
 * Destructor for DataWrapper.
 * Cleans up dynamically allocated resources.
 */
DataWrapper::~DataWrapper() {
    delete lfb;
    delete blfb;
    delete mouseInputStream;
    delete keyDecoder;
    delete mouseClicks;
    delete currentInput;
    delete flags;
    delete mHandler;
    delete layers;
    delete history;
    delete settings;
    delete guiLayers;
}
