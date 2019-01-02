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

#include <kernel/events/storage/StorageAddEvent.h>
#include <kernel/events/storage/StorageRemoveEvent.h>
#include <filesystem/RamFs/memory/KernelHeapNode.h>
#include <filesystem/RamFs/memory/IOMemoryNode.h>
#include <filesystem/RamFs/memory/PhysicalMemoryNode.h>
#include <filesystem/RamFs/memory/PagingAreaNode.h>
#include <kernel/log/FileAppender.h>
#include <kernel/log/Logger.h>
#include <filesystem/RamFs/graphics/CurrentResolutionNode.h>
#include <devices/input/Keyboard.h>
#include <kernel/services/InputService.h>
#include <lib/multiboot/Structure.h>
#include <lib/file/tar/Archive.h>
#include <filesystem/TarArchive/TarArchiveDriver.h>
#include "FileSystem.h"
#include "lib/file/Directory.h"
#include "filesystem/RamFs/graphics/GraphicsVendorNameNode.h"
#include "filesystem/RamFs/graphics/GraphicsDeviceNameNode.h"
#include "filesystem/RamFs/graphics/GraphicsResolutionsNode.h"
#include "filesystem/RamFs/graphics/GraphicsMemoryNode.h"
#include "filesystem/RamFs/RamFsDriver.h"
#include "filesystem/RamFs/storage/StorageNode.h"
#include "filesystem/RamFs/PciNode.h"
#include "filesystem/RamFs/StdoutNode.h"
#include "filesystem/RamFs/StderrNode.h"

Logger &FileSystem::log = Logger::get("FILESYSTEM");

FileSystem::FileSystem() {
    eventBus = Kernel::getService<EventBus>();
    storageService = Kernel::getService<StorageService>();
}

FileSystem::~FileSystem() {
    eventBus->unsubscribe(*this, StorageAddEvent::TYPE);
    eventBus->unsubscribe(*this, StorageRemoveEvent::TYPE);
}

String FileSystem::parsePath(const String &path) {
    Util::Array<String> token = path.split(FileSystem::SEPARATOR);
    Util::List<String> *parsedToken = new Util::ArrayList<String>();

    for (const String &string : token) {
        if (string == ".") {
            continue;
        } else if (string == "..") {
            if(!parsedToken->isEmpty()) {
                parsedToken->remove(parsedToken->size() - 1);
            }
        } else {
            parsedToken->add(*new String(string));
        }
    }

    if (parsedToken->isEmpty()) {
        delete parsedToken;
        return "";
    }

    String parsedPath = FileSystem::SEPARATOR + String::join(FileSystem::SEPARATOR, parsedToken->toArray());
    return parsedPath;
}

FsDriver *FileSystem::getMountedDriver(String &path) {
    if(!path.endsWith(FileSystem::SEPARATOR)) {
        path += FileSystem::SEPARATOR;
    }

    String ret;

    for(const String &currentString : mountPoints.keySet()) {
        if (path.beginsWith(currentString)) {
            if(currentString.length() > ret.length()) {
                ret = currentString;
            }
        }
    }

    if(ret.isEmpty()) {
        return nullptr;
    }

    path = path.substring(ret.length(), path.length() - 1);
    return mountPoints.get(ret);
}

void FileSystem::init() {
    log.trace("Unmounting initial ramdisk");

    for(const String &path : mountPoints.keySet()) {
        delete mountPoints.get(path);
    }

    mountPoints.clear();

    FsDriver::registerPrototype(new RamFsDriver());
    // Mount root-device
    StorageDevice *rootDevice = storageService->findRootDevice();

    if(rootDevice == nullptr) {
        // No root-device found -> Mount RAM-Device
        log.warn("No root-partition found. Mounting RamFs to /");

        mount("", "/", "RamFsDriver");
    } else {
        log.info("Found root-partition %s. Mounting %s to /", static_cast<const char*>(rootDevice->getName()),
                static_cast<const char*>(rootDevice->getName()));

        if(mount(rootDevice->getName(), "/", "FatDriver") != SUCCESS) {
            log.warn("Unable to mount root-partition. Mounting RamFs to /");

            mount("", "/", "RamFsDriver");
        }
    }

    log.trace("Remounting initial ramdisk to /initrd/");

    createDirectory("/initrd");
    mountInitRamdisk("/initrd");

    log.trace("Initializing /dev");

    // Initialize dev-Directory
    Directory *dev = Directory::open("/dev");
    if(dev == nullptr) {
        createDirectory("/dev");
    } else {
        delete dev;
    }

    mount("", "/dev", "RamFsDriver");

    // Create directory for StorageNodes.
    createDirectory("/dev/storage");

    // Create directory for ports
    createDirectory("/dev/ports");

    // Add Video-nodes to dev-Directory
    createDirectory("/dev/video");
    createDirectory("/dev/video/text");
    createDirectory("/dev/video/lfb");
    addVirtualNode("/dev/video/text", new GraphicsVendorNameNode(GraphicsNode::TEXT));
    addVirtualNode("/dev/video/text", new GraphicsDeviceNameNode(GraphicsNode::TEXT));
    addVirtualNode("/dev/video/text", new GraphicsMemoryNode(GraphicsNode::TEXT));
    addVirtualNode("/dev/video/text", new GraphicsResolutionsNode(GraphicsNode::TEXT));
    addVirtualNode("/dev/video/text", new CurrentResolutionNode(GraphicsNode::TEXT));
    addVirtualNode("/dev/video/lfb", new GraphicsVendorNameNode(GraphicsNode::LINEAR_FRAME_BUFFER));
    addVirtualNode("/dev/video/lfb", new GraphicsDeviceNameNode(GraphicsNode::LINEAR_FRAME_BUFFER));
    addVirtualNode("/dev/video/lfb", new GraphicsMemoryNode(GraphicsNode::LINEAR_FRAME_BUFFER));
    addVirtualNode("/dev/video/lfb", new GraphicsResolutionsNode(GraphicsNode::LINEAR_FRAME_BUFFER));
    addVirtualNode("/dev/video/lfb", new CurrentResolutionNode(GraphicsNode::LINEAR_FRAME_BUFFER));

    // Add Memory-nodes to dev-directory
    createDirectory("/dev/memory");
    addVirtualNode("/dev/memory", new KernelHeapNode());
    addVirtualNode("/dev/memory", new IOMemoryNode());
    addVirtualNode("/dev/memory", new PhysicalMemoryNode());
    addVirtualNode("/dev/memory", new PagingAreaNode());

    // Create folder for network files
    createDirectory("/dev/network");

    // Add PCI-node to dev-Directory
    addVirtualNode("/dev", new PciNode());

    // Add syslog file to dev-Ddrectory
    createFile("/dev/syslog");

    // Add StdStream-nodes to dev-Directory
    addVirtualNode("/dev", new StdoutNode());
    addVirtualNode("/dev", new StderrNode());

    // Subscribe to Storage-Events from the EventBus
    eventBus->subscribe(*this, StorageAddEvent::TYPE);
    eventBus->subscribe(*this, StorageRemoveEvent::TYPE);

    FileAppender *fileAppender = new FileAppender(File::open("/dev/syslog", "a+"));

    Logger::addAppender(fileAppender);
}

void FileSystem::mountInitRamdisk(const String &path) {
    Multiboot::ModuleInfo info = Multiboot::Structure::getModule("initrd");

    Address address(info.start);

    Tar::Archive &archive = Tar::Archive::from(address);

    FsDriver *tarDriver = new TarArchiveDriver(&archive);

    if(!path.endsWith(FileSystem::SEPARATOR)) {
        mountPoints.put(path + FileSystem::SEPARATOR, tarDriver);
    } else {
        mountPoints.put(path, tarDriver);
    }
}

uint32_t FileSystem::addVirtualNode(const String &path, VirtualNode *node) {

    fsLock.acquire();

    String parsedPath = parsePath(path);

    auto *driver = getMountedDriver(parsedPath);

    if(driver->getTypeName() != "RamFsDriver") {
        fsLock.release();

        return ADDING_VIRTUAL_NODE_FAILED;
    }

    bool ret = ((RamFsDriver*) driver)->addNode(parsedPath, node);

    fsLock.release();

    return ret ? SUCCESS : ADDING_VIRTUAL_NODE_FAILED;
}

uint32_t FileSystem::createFilesystem(const String &devicePath, const String &fsType) {
    // Check if device-node exists
    FsNode *deviceNode = getNode(devicePath);
    if(deviceNode == nullptr) {
        return DEVICE_NOT_FOUND;
    }

    // Get device
    StorageDevice *disk = storageService->getDevice(deviceNode->getName());
    if(disk == nullptr) {
        return DEVICE_NOT_FOUND;
    }

    fsLock.acquire();

    // Create temporary driver
    FsDriver *tmpDriver = (FsDriver*) FsDriver::createInstance(fsType);

    // Format device
    bool ret = tmpDriver->createFs(disk);

    fsLock.release();

    delete tmpDriver;
    return ret ? SUCCESS : FORMATTING_FAILED;
}

uint32_t FileSystem::mount(const String &devicePath, const String &targetPath, const String &fsType) {
    StorageDevice *disk = nullptr;
    String parsedDevicePath = parsePath(devicePath);

    if(fsType != TYPE_RAM_FS) {
        // Check if device-node exists
        FsNode *deviceNode = getNode(parsedDevicePath);

        if(deviceNode == nullptr) {
            // Device node is non-existent,
            // Check if devicePath is the name of a storage device.
            disk = storageService->getDevice(devicePath);
            if(disk == nullptr) {
                return DEVICE_NOT_FOUND;
            }
        } else {
            disk = storageService->getDevice(deviceNode->getName());
            if(disk == nullptr) {
                return DEVICE_NOT_FOUND;
            }
        }
    }

    String parsedPath = parsePath(targetPath) + FileSystem::SEPARATOR;
    FsNode *targetNode = getNode(parsedPath);

    if(targetNode == nullptr) {
        if(mountPoints.size() != 0) {
            return FILE_NOT_FOUND;
        }
    } else {
        delete targetNode;
    }

    fsLock.acquire();

    if(mountPoints.containsKey(parsedPath)) {
        fsLock.release();
        return MOUNT_TARGET_ALREADY_USED;
    }

    FsDriver *driver = (FsDriver*) FsDriver::createInstance(fsType);

    if(!driver->mount(disk)) {
        fsLock.release();
        delete driver;
        return MOUNTING_FAILED;
    }

    mountPoints.put(parsedPath, driver);

    fsLock.release();
    return SUCCESS;
}

uint32_t FileSystem::unmount(const String &path) {
    String parsedPath = parsePath(path) + FileSystem::SEPARATOR;
    FsNode *targetNode = getNode(parsedPath);

    if(targetNode == nullptr) {
        if(path != "/") {
            return FILE_NOT_FOUND;
        }
    } else {
        delete targetNode;
    }

    fsLock.acquire();

    for(const String &key : mountPoints.keySet()) {
        if(key.beginsWith(parsedPath)) {
            if(key != parsedPath) {
                fsLock.release();
                return SUBDIRECTORY_CONTAINS_MOUNT_POINT;
            }
        }
    }

    if(mountPoints.containsKey(parsedPath)) {
        delete mountPoints.get(parsedPath);
        mountPoints.remove(parsedPath);

        fsLock.release();
        return SUCCESS;
    }

    fsLock.release();

    return NOTHING_MOUNTED_AT_PATH;
}

FsNode *FileSystem::getNode(const String &path) {
    String parsedPath = parsePath(path);

    fsLock.acquire();

    FsDriver *driver = getMountedDriver(parsedPath);

    if(driver == nullptr) {
        fsLock.release();
        return nullptr;
    }
    
    FsNode *ret = driver->getNode(parsedPath);

    fsLock.release();

    return ret;
}

uint32_t FileSystem::createFile(const String &path) {
    String parsedPath = parsePath(path);

    fsLock.acquire();

    FsDriver *driver = getMountedDriver(parsedPath);

    if(driver == nullptr) {
        fsLock.release();
        return FILE_NOT_FOUND;
    }
    
    bool ret = driver->createNode(parsedPath, FsNode::REGULAR_FILE);

    fsLock.release();

    return ret ? SUCCESS : CREATING_FILE_FAILED;
}

uint32_t FileSystem::createDirectory(const String &path) {
    String parsedPath = parsePath(path);
    fsLock.acquire();

    FsDriver *driver = getMountedDriver(parsedPath);

    if(driver == nullptr) {
        fsLock.release();
        return FILE_NOT_FOUND;
    }

    bool ret = driver->createNode(parsedPath, FsNode::DIRECTORY_FILE);

    fsLock.release();

    return ret ? SUCCESS : CREATING_DIRECTORY_FAILED;
}

uint32_t FileSystem::deleteFile(const String &path) {
    String parsedPath = parsePath(path);

    fsLock.acquire();

    for(const String &key : mountPoints.keySet()) {
        if(key.beginsWith(parsedPath)) {
            fsLock.release();
            return SUBDIRECTORY_CONTAINS_MOUNT_POINT;
        }
    }

    FsDriver *driver = getMountedDriver(parsedPath);

    if(driver == nullptr) {
        fsLock.release();
        return FILE_NOT_FOUND;
    }
    
    bool ret = driver->deleteNode(parsedPath);

    fsLock.release();

    return ret ? SUCCESS : DELETING_FILE_FAILED;
}

void FileSystem::onEvent(const Event &event) {
    if(event.getType() == StorageAddEvent::TYPE) {
        StorageDevice *device = ((StorageAddEvent &) event).getDevice();

        deleteFile("/dev/storage/" + device->getName());
        addVirtualNode("/dev/storage/", new StorageNode(device));
    } else if(event.getType() == StorageAddEvent::TYPE) {
        String deviceName = ((StorageRemoveEvent &) event).getDeviceName();

        deleteFile("/dev/storage/" + deviceName);
    }
}
