/*
 * Copyright (C) 2018-2024 Heinrich-Heine-Universitaet Duesseldorf,
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
 */

#include "lib/util/base/Address.h"
#include "LinearFrameBufferNode.h"
#include "lib/util/graphic/LinearFrameBuffer.h"
#include "kernel/service/Service.h"
#include "kernel/service/MemoryService.h"
#include "device/graphic/VesaBiosExtensions.h"
#include "kernel/service/InformationService.h"
#include "kernel/multiboot/Multiboot.h"

namespace Device::Graphic {

LinearFrameBufferNode::LinearFrameBufferNode(const Util::String &name, const Util::Graphic::LinearFrameBuffer &lfb) : Filesystem::Memory::StringNode(name),
        physicalAddress(Kernel::Service::getService<Kernel::MemoryService>().getPhysicalAddress(reinterpret_cast<void*>(lfb.getBuffer().get()))),
        resolutionX(lfb.getResolutionX()), resolutionY(lfb.getResolutionY()), colorDepth(lfb.getColorDepth()), pitch(lfb.getPitch()) {
    const auto &multiboot = Kernel::Service::getService<Kernel::InformationService>().getMultibootInformation();
    if (multiboot.getKernelOption("vbe", "true") == "true" && Device::Graphic::VesaBiosExtensions::isAvailable()) {
        vbe = Device::Graphic::VesaBiosExtensions::initialize();
    }
}

LinearFrameBufferNode::~LinearFrameBufferNode() {
    delete vbe;
}

Util::String LinearFrameBufferNode::getString() {
    auto buffer = Util::String::format("%u\n%ux%u@%u\n%u\n", physicalAddress, resolutionX, resolutionY, colorDepth, pitch);
    if (vbe != nullptr) {
        const auto &deviceInfo = vbe->getDeviceInfo();

        buffer += Util::String::format("OEM string: %s\n", deviceInfo.getOemString());
        if (deviceInfo.vbeVersion >= 0x0200) {
            buffer += Util::String::format("Vendor: %s\nDevice: %s\nRevision: %s\nSupported modes:\n", deviceInfo.getVendorName(), deviceInfo.getProductName(), deviceInfo.getProductRevision());
        }

        for (const auto &mode : vbe->getSupportedModes()) {
            buffer += Util::String::format("%ux%u@%u\n", mode.resolutionX, mode.resolutionY, mode.colorDepth);
        }
    }

    return buffer;
}

bool LinearFrameBufferNode::control(uint32_t request, const Util::Array<uint32_t> &parameters) {
    switch (request) {
        case Util::Graphic::LinearFrameBuffer::SET_RESOLUTION: {
            if (vbe == nullptr || parameters.length() < 3) {
                return false;
            }

            const auto &mode = vbe->findMode(parameters[0], parameters[1], parameters[2]);
            Device::Graphic::VesaBiosExtensions::setMode(mode.modeNumber);

            physicalAddress = reinterpret_cast<void*>(mode.physicalAddress);
            resolutionX = mode.resolutionX;
            resolutionY = mode.resolutionY;
            colorDepth = mode.colorDepth;
            pitch = mode.pitch;

            return true;
        }
        default:
            return false;
    }
}

}