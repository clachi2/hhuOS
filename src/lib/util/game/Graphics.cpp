/*
 * Copyright (C) 2018-2023 Heinrich-Heine-Universitaet Duesseldorf,
 * Institute of Computer Science, Department Operating Systems
 * Burak Akguel, Christian Gesse, Fabian Ruhland, Filip Krakowski, Michael Schoettner
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
 *
 * The game engine is based on a bachelor's thesis, written by Malte Sehmer.
 * The original source code can be found here: https://github.com/Malte2036/hhuOS
 */

#include "lib/util/graphic/Fonts.h"
#include "Graphics.h"
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "lib/util/math/Vector2D.h"
#include "Game.h"
#include "lib/util/game/Camera.h"
#include "lib/util/graphic/Image.h"
#include "lib/util/base/Address.h"
#include "lib/util/game/Scene.h"
#include "lib/util/math/Math.h"

namespace Util {
namespace Graphic {
class Font;
}  // namespace Graphic
}  // namespace Util

namespace Util::Game {

Graphics::Graphics(const Graphic::LinearFrameBuffer &lfb, Game &game) :
        game(game), lfb(lfb), pixelDrawer(Graphics::lfb), lineDrawer(pixelDrawer), stringDrawer(pixelDrawer),
        transformation((lfb.getResolutionX() > lfb.getResolutionY() ? lfb.getResolutionY() : lfb.getResolutionX()) / 2),
        offsetX(transformation + (lfb.getResolutionX() > lfb.getResolutionY() ? (lfb.getResolutionX() - lfb.getResolutionY()) / 2 : 0)),
        offsetY(transformation + (lfb.getResolutionY() > lfb.getResolutionX() ? (lfb.getResolutionY() - lfb.getResolutionX()) / 2 : 0)) {}

/***** Basic functions to draw directly on the screen ******/

void Graphics::drawString(const Graphic::Font &font, uint16_t x, uint16_t y, const char *string) const {
    stringDrawer.drawString(font, x, y, string, color, Util::Graphic::Colors::INVISIBLE);
}

void Graphics::drawString(uint16_t x, uint16_t y, const char *string) const {
    drawString(Graphic::Fonts::TERMINAL_FONT, x, y, string);
}

void Graphics::drawString(uint16_t x, uint16_t y, const String &string) const {
    drawString(x, y, static_cast<const char*>(string));
}

void Graphics::drawStringSmall(uint16_t x, uint16_t y, const char *string) const {
    drawString(Graphic::Fonts::TERMINAL_FONT_SMALL, x, y, string);
}

void Graphics::drawStringSmall(uint16_t x, uint16_t y, const String &string) const {
    drawStringSmall(x, y, static_cast<const char*>(string));
}

/***** 2D drawing functions, respecting the camera position *****/

void Graphics::drawLine2D(const Math::Vector2D &from, const Math::Vector2D &to) const {
    auto &cameraPosition = game.getCurrentScene().getCamera().getPosition();
    lineDrawer.drawLine(static_cast<int32_t>((from.getX() - cameraPosition.getX()) * transformation + offsetX),
                        static_cast<int32_t>((-from.getY() + cameraPosition.getY()) * transformation + offsetY),
                        static_cast<int32_t>((to.getX() - cameraPosition.getX()) * transformation + offsetX),
                        static_cast<int32_t>((-to.getY() + cameraPosition.getY()) * transformation + offsetY), color);
}

void Graphics::drawPolygon2D(const Array<Math::Vector2D> &vertices) const {
    for (uint32_t i = 0; i < vertices.length() - 1; i++) {
        drawLine2D(vertices[i], vertices[i + 1]);
    }

    drawLine2D(vertices[vertices.length() - 1], vertices[0]);
}

void Graphics::drawSquare2D(const Math::Vector2D &position, double size) const {
    drawRectangle2D(position, size, size);
}

void Graphics::drawRectangle2D(const Math::Vector2D &position, double width, double height) const {
    auto x = position.getX();
    auto y = position.getY();

    drawLine2D(position, Math::Vector2D(x + width, y));
    drawLine2D(Math::Vector2D(x, y - height), Math::Vector2D(x + width, y - height));
    drawLine2D(position, Math::Vector2D(x, y - height));
    drawLine2D(Math::Vector2D(x + width, y), Math::Vector2D(x + width, y - height));
}

void Graphics::fillSquare2D(const Math::Vector2D &position, double size) const {
    fillRectangle2D(position, size, size);
}

void Graphics::fillRectangle2D(const Math::Vector2D &position, double width, double height) const {
    auto &cameraPosition = game.getCurrentScene().getCamera().getPosition();
    auto startX = static_cast<int32_t>((position.getX() - cameraPosition.getX()) * transformation + offsetX);
    auto endX = static_cast<int32_t>((position.getX() + width - cameraPosition.getX()) * transformation + offsetX);
    auto startY = static_cast<int32_t>((-position.getY() + cameraPosition.getY()) * transformation + offsetY);
    auto endY = static_cast<int32_t>((-position.getY() + height + cameraPosition.getY()) * transformation + offsetY);

    for (int32_t i = startY; i < endY; i++) {
        lineDrawer.drawLine(startX, i, endX, i, color);
    }
}

void Graphics::drawString2D(const Graphic::Font &font, const Math::Vector2D &position, const char *string) const {
    auto &cameraPosition = game.getCurrentScene().getCamera().getPosition();
    stringDrawer.drawString(font, static_cast<int32_t>((position.getX() - cameraPosition.getX()) * transformation + offsetX), static_cast<int32_t>((-position.getY() + cameraPosition.getY()) * transformation + offsetY), string, color, Util::Graphic::Colors::INVISIBLE);
}

void Graphics::drawString2D(const Math::Vector2D &position, const char *string) const {
    drawString2D(Graphic::Fonts::TERMINAL_FONT, position, string);
}

void Graphics::drawString2D(const Math::Vector2D &position, const String &string) const {
    drawString2D(position, static_cast<const char*>(string));
}

void Graphics::drawStringSmall2D(const Math::Vector2D &position, const char *string) const {
    drawString2D(Graphic::Fonts::TERMINAL_FONT_SMALL, position, string);
}

void Graphics::drawStringSmall2D(const Math::Vector2D &position, const String &string) const {
    drawStringSmall2D(position, static_cast<const char*>(string));
}

void Graphics::drawImage2D(const Math::Vector2D &position, const Graphic::Image &image, bool flipX) const {
    auto &cameraPosition = game.getCurrentScene().getCamera().getPosition();
    auto pixelBuffer = image.getPixelBuffer();
    auto xFlipOffset = flipX ? image.getWidth() - 1 : 0;
    auto xPixelOffset = static_cast<int32_t>((position.getX() - cameraPosition.getX()) * transformation + offsetX);
    auto yPixelOffset = static_cast<int32_t>((-position.getY() + cameraPosition.getY()) * transformation + offsetY);

    if (xPixelOffset + image.getWidth() < 0 || xPixelOffset > lfb.getResolutionX() ||
        yPixelOffset - image.getHeight() > lfb.getResolutionY() || yPixelOffset < 0) {
        return;
    }

    for (int32_t i = 0; i < image.getHeight(); i++) {
        for (int32_t j = 0; j < image.getWidth(); j++) {
            pixelDrawer.drawPixel(xPixelOffset + xFlipOffset + (flipX ? -1 : 1) * j, yPixelOffset - i, pixelBuffer[i * image.getWidth() + j]);
        }
    }
}

// Based on https://en.wikipedia.org/wiki/3D_projection#Perspective_projection
Math::Vector2D Graphics::projectPoint(const Math::Vector3D &v, const Math::Vector3D &camT, const Math::Vector3D &camRr) const {
    // Objects are visible between -1 and 1 on both axes, returning {-2, -2} for example means a point isn't rendered
    auto fov = 1.3;

    Math::Vector3D unitV = {0, 0, 1};
    auto r = camRr;
    auto lineDir = unitV.rotate(r);
    auto distToClosestPointOnLine = (v - camT) * lineDir;

    if (distToClosestPointOnLine <= 0) {
        return {-2, -2};
    }

    // Convert deg to rad
    auto camR = camRr * (3.1415 / 180);

    double x = camR.getX();
    double y = camR.getY();
    double z = camR.getZ();

    double sinX = Util::Math::sine(x);
    double cosX = Util::Math::cosine(x);
    double sinY = Util::Math::sine(y);
    double cosY = Util::Math::cosine(y);
    double sinZ = Util::Math::sine(z);
    double cosZ = Util::Math::cosine(z);

    Math::Matrix3x3 rot = {
            cosY * cosZ, cosY * sinZ, -sinY,
            sinX * sinY * cosZ - cosX * sinZ, sinX * sinY * sinZ + cosX * cosZ, sinX * cosY,
            cosX * sinY * cosZ + sinX * sinZ, cosX * sinY * sinZ - sinX * cosZ, cosX * cosY
    };

    Math::Vector3D d = rot * (v - camT);
    Math::Vector3D e = {0, 0, fov};
    double a = 1;
    if (d.getZ() != 0) {
        a = e.getZ() / d.getZ();
    }

    return {
            a * d.getX() + e.getX(),
            a * d.getY() + e.getY()
    };
}

void Graphics::drawLine3D(const Math::Vector3D &from, const Math::Vector3D &to) const {
    auto &camera = game.getCurrentScene().getCamera();
    Util::Math::Vector2D v1 = projectPoint(from, camera.getPosition(), camera.getRotation());
    Util::Math::Vector2D v2 = projectPoint(to, camera.getPosition(), camera.getRotation());

    // lines aren't drawn if one of the points is outside the camera view (range (-1, 1))
    if (v1.getX() < -1 || v1.getX() > 1 || v1.getY() < -1 || v1.getY() > 1) {
        if (v2.getX() < -1 || v2.getX() > 1 || v2.getY() < -1 || v2.getY() > 1) {
            return;
        }
    }

    // map the points of range (-1, 1) to actual screen coordinates
    auto x1 = static_cast<int32_t>((v1.getX() + 1) * (lfb.getResolutionX() / 2.0));
    auto y1 = static_cast<int32_t>(lfb.getResolutionY() - (v1.getY() + 1) * (lfb.getResolutionY() / 2.0));
    auto x2 = static_cast<int32_t>((v2.getX() + 1) * (lfb.getResolutionX() / 2.0));
    auto y2 = static_cast<int32_t>(lfb.getResolutionY() - (v2.getY() + 1) * (lfb.getResolutionY() / 2.0));

    lineDrawer.drawLine(x1, y1, x2, y2, color);
}

void Graphics::drawModel(const Array<Math::Vector3D> &vertices, const Array<Math::Vector2D> &edges) const {
    auto numEdges = edges.length();

    for (uint32_t i = 0; i < numEdges; i++) {
        auto edge = edges[i];
        auto x = static_cast<int32_t>(edge.getX());
        auto y = static_cast<int32_t>(edge.getY());
        auto numVertices = static_cast<int32_t>(vertices.length());

        // Do not draw edges pointing to out of bounds vertices
        if (x < 0 || x >= numVertices || y < 0 || y >= numVertices) {
            continue;
        }

        // Do not draw edges where the vertices are the same
        if (x == y) {
            continue;
        }

        drawLine3D(vertices[x], vertices[y]);
    }
}

void Graphics::show() const {
    auto &cameraPosition = game.getCurrentScene().getCamera().getPosition();
    lfb.flush();

    if (backgroundBuffer == nullptr) {
        lfb.clear();
    } else if (Math::Vector2D(cameraPosition.getX(), cameraPosition.getY()) == Math::Vector2D(0, 0)) {
        auto source = Address<uint32_t>(backgroundBuffer);
        lfb.getBuffer().copyRange(source, lfb.getResolutionY() * lfb.getPitch());
    } else {
        auto pitch = lfb.getPitch();
        auto colorDepthDivisor = (lfb.getColorDepth() == 15 ? 16 : lfb.getColorDepth()) / 8;
        auto xOffset = static_cast<uint32_t>(game.getCurrentScene().getCamera().getPosition().getX() * pitch / 4) % pitch;
        xOffset -= xOffset % colorDepthDivisor;

        for (uint32_t i = 0; i < lfb.getResolutionY(); i++) {
            auto yOffset = pitch * i;

            auto source = Address<uint32_t>(backgroundBuffer + yOffset + xOffset);
            auto target = lfb.getBuffer().add(yOffset);
            target.copyRange(source, pitch - xOffset);

            source = Address<uint32_t>(backgroundBuffer + yOffset);
            target = lfb.getBuffer().add(yOffset + (pitch - xOffset));
            target.copyRange(source, pitch - (pitch - xOffset));
        }
    }
}

void Graphics::setColor(const Graphic::Color &color) {
    Graphics::color = color;
}

Graphic::Color Graphics::getColor() const {
    return color;
}

void Graphics::saveCurrentStateAsBackground() {
    if (backgroundBuffer == nullptr) {
        backgroundBuffer = new uint8_t[lfb.getPitch() * lfb.getResolutionY()];
    }

    Address<uint32_t>(backgroundBuffer).copyRange(lfb.getBuffer(), lfb.getPitch() * lfb.getResolutionY());
}

void Graphics::clear(const Graphic::Color &color) {
    if (color == Util::Graphic::Colors::BLACK) {
        lfb.clear();
    } else {
        for (uint32_t i = 0; i < lfb.getResolutionX(); i++) {
            for (uint32_t j = 0; j < lfb.getResolutionY(); j++) {
                pixelDrawer.drawPixel(i, j, color);
            }
        }
    }
}

}