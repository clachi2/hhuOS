/*
 * Copyright (C) 2018 Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
 * Heinrich-Heine University
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#ifndef __LinearFrameBuffer_include__
#define __LinearFrameBuffer_include__

#include <cstdint>
#include "Color.h"

namespace Util {

/**
 * Wraps a chunk of memory, that can be used as a linear frame buffer.
 */
class LinearFrameBuffer {

public:
    /**
     * Constructor.
     *
     * @param address The buffer address
     * @param resolutionX The horizontal resolution
     * @param resolutionY The vertical resolution
     * @param bitsPerPixel The color depth
     * @param pitch The pitch
     */
    LinearFrameBuffer(void *address, uint16_t resolutionX, uint16_t resolutionY, uint8_t bitsPerPixel, uint16_t pitch);

    /**
     * Assignment operator.
     */
     LinearFrameBuffer& operator=(const LinearFrameBuffer &other) = delete;

    /**
     * Copy constructor.
     */
    LinearFrameBuffer(const LinearFrameBuffer &copy) = delete;

    /**
     * Destructor.
     */
    ~LinearFrameBuffer() = default;

    /**
     * Get the horizontal resolution.
     *
     * @return The horizontal resolution
     */
    [[nodiscard]] uint16_t getResX() const;

    /**
     * Get the vertical resolution.
     *
     * @return The vertical resolution
     */
    [[nodiscard]] uint16_t getResY() const;

    /**
     * Get the color depth.
     *
     * @return The color depth
     */
    [[nodiscard]] uint8_t getDepth() const;

    /**
     * Get the buffer's pitch.
     *
     * @return The pitch
     */
    [[nodiscard]] uint16_t getPitch() const;

    /**
     * Get the buffer address.
     *
     * @return The buffer address
     */
    [[nodiscard]] uint8_t* getBuffer() const;

    /**
     * Read the color of a pixel at a given position.
     *
     * @param x The x-coordinate
     * @param y The y-coordinate
     * @param color A reference to the variable, that the pixel's color will be written to
     */
    Color readPixel(uint16_t x, uint16_t y);

private:
    uint8_t *buffer = nullptr;

    uint16_t resolutionX = 0;
    uint16_t resolutionY = 0;
    uint8_t bitsPerPixel = 0;
    uint16_t pitch = 0;

};

}

#endif