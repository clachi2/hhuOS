/*
 * Copyright (C) 2018-2021 Heinrich-Heine-Universitaet Duesseldorf,
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

#ifndef HHUOS_MANAGEMENT_H
#define HHUOS_MANAGEMENT_H

#include <cstdint>

namespace Util::Memory {

class Management {

public:
    /**
     * Default Constructor.
     * Deleted, as this class has only static members.
     */
    Management() = delete;

    /**
     * Copy constructor.
     */
    Management(const Management &other) = delete;

    /**
     * Assignment operator.
     */
    Management &operator=(const Management &other) = delete;

    /**
     * Destructor.
     */
    ~Management() = default;

    static void* alloc(uint32_t size);

    static void free(void *pointer);

    static void* alignedAlloc(uint32_t size, uint32_t alignment);

    static void alignedFree(void *pointer, uint32_t alignment);

};

}

#endif