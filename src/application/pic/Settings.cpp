//
// Created by Rafael Reip on 02.12.24.
//

#include "Settings.h"

#include "lib/util/io/stream/BufferedInputStream.h"
#include "lib/util/io/stream/FileInputStream.h"

#include "MessageHandler.h"

/**
 * \brief Constructor for the Settings class.
 *
 * \param mHandler Pointer to a MessageHandler object used for logging messages.
 */
Settings::Settings(MessageHandler *mHandler) {
    this->mHandler = mHandler;
    this->path = "/pic/settings";

    resetToDefault();
    loadFromFile();
    saveToFile();
}

Settings::~Settings() = default;

/**
 * \brief Resets all settings to their default values.
 *
 * This method initializes all settings to their default values and logs it to the message handler.
 */
void Settings::resetToDefault() {
    checkeredBackground = true;
    optimizeRendering = true;
    currentLayerOverlay = true;
    activateHotkeys = true;
    showFPS = false;
    textCaptureAfterUse = false;
    resetValuesAfterConfirm = false;
    useBufferedBuffer = false;
    showMouseHelper = true;
    mHandler->addMessage("Settings restored to default values");
}

/**
 * \brief Loads settings from a file.
 *
 * This method reads settings from a file specified by the path member variable.
 * It logs errors if the path is invalid, the file does not exist, or the file is a directory.
 * The settings are read line by line and parsed into the corresponding member variables.
 */
void Settings::loadFromFile() {
    if (Util::String(path).length() == 0) {
        mHandler->addMessage("Settings::loadFromFile Error: No path given");
        return;
    }
    auto file = Util::Io::File(path);
    if (!file.exists()) {
        mHandler->addMessage("Settings::loadFromFile Error: File not found: " + path);
        return;
    }
    if (file.isDirectory()) {
        mHandler->addMessage("Settings::loadFromFile Error: File is a directory: " + path);
        return;
    }

    auto fileStream = Util::Io::FileInputStream(file);
    auto bufferedStream = Util::Io::BufferedInputStream(fileStream);
    auto &stream = (file.getType() == Util::Io::File::REGULAR) ? static_cast<Util::Io::InputStream &>(bufferedStream)
                                                               : static_cast<Util::Io::InputStream &>(fileStream);
    bool eof = false;
    auto line = stream.readLine(eof);
    while (!eof) {
        if (line.length() > 0) {
            auto parts = line.split(" ");
            if (parts[0] == "checkeredBackground") checkeredBackground = parts[1] == "true";
            else if (parts[0] == "optimizeRendering") optimizeRendering = parts[1] == "true";
            else if (parts[0] == "currentLayerOverlay") currentLayerOverlay = parts[1] == "true";
            else if (parts[0] == "activateHotkeys") activateHotkeys = parts[1] == "true";
            else if (parts[0] == "showFPS") showFPS = parts[1] == "true";
            else if (parts[0] == "textCaptureAfterUse") textCaptureAfterUse = parts[1] == "true";
            else if (parts[0] == "resetValuesAfterConfirm") resetValuesAfterConfirm = parts[1] == "true";
            else if (parts[0] == "useBufferedBuffer") useBufferedBuffer = parts[1] == "true";
            else if (parts[0] == "showMouseHelper") showMouseHelper = parts[1] == "true";
            else mHandler->addMessage("Settings Error: Unknown setting: " + parts[0]);
        }
        line = stream.readLine(eof);
    }
    mHandler->addMessage("Settings loaded from: " + path);
}

/**
 * \brief Saves the current settings to a file.
 *
 * This method writes the current settings to a file specified by the path member variable.
 * It creates the directory if it does not exist and logs errors if the path is invalid,
 * the file cannot be opened, or the directory cannot be created.
 */
void Settings::saveToFile() {
    if (Util::String(path).length() == 0) {
        mHandler->addMessage("Settings::saveToFile Error: No path given");
        return;
    }
    auto picFolder = Util::Io::File("pic");
    if (!picFolder.exists()) {
        mHandler->addMessage("Settings::saveToFile: Creating directory: pic");
        auto success = picFolder.create(Util::Io::File::DIRECTORY);
        if (!success) {
            mHandler->addMessage("Settings::saveToFile Error: Could not create directory: pic");
            return;
        }
    }

    FILE *file = fopen(path.operator const char *(), "w");
    if (!file) {
        mHandler->addMessage("Settings::saveToFile Error: Could not open file: " + path);
        return;
    }
    fputs(Util::String::format("checkeredBackground %s\n", checkeredBackground ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("optimizeRendering %s\n", optimizeRendering ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("currentLayerOverlay %s\n", currentLayerOverlay ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("activateHotkeys %s\n", activateHotkeys ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("showFPS %s\n", showFPS ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("textCaptureAfterUse %s\n", textCaptureAfterUse ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("resetValuesAfterConfirm %s\n", resetValuesAfterConfirm ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("useBufferedBuffer %s\n", useBufferedBuffer ? "true" : "false").operator const char *(), file);
    fputs(Util::String::format("showMouseHelper %s\n", showMouseHelper ? "true" : "false").operator const char *(), file);
    fflush(file);
    fclose(file);
    mHandler->addMessage("Settings saved to: " + path);
}
