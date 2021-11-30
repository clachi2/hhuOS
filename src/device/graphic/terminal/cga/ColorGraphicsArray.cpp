#include <kernel/paging/MemLayout.h>
#include <kernel/system/System.h>
#include "ColorGraphicsArray.h"

namespace Device::Graphic {

ColorGraphicsArray::ColorGraphicsArray(uint16_t columns, uint16_t rows) : Terminal(columns, rows),
        cgaMemory(Kernel::System::getMemoryService().mapIO(CGA_START_ADDRESS, columns * rows * 2)), indexPort(INDEX_PORT_ADDRESS), dataPort(DATA_PORT_ADDRESS) {
    ColorGraphicsArray::clear(Util::Graphic::Colors::BLACK);

    // Set cursor shape
    indexPort.writeByte(CURSOR_START_INDEX);
    dataPort.writeByte(0x00);
    indexPort.writeByte(CURSOR_END_INDEX);
    dataPort.writeByte(0x1F);
}

void ColorGraphicsArray::putChar(char c, const Util::Graphic::Color &foregroundColor, const Util::Graphic::Color &backgroundColor) {
    uint16_t position = (currentRow * getColumns() + currentColumn) * BYTES_PER_CHARACTER;
    uint8_t colorAttribute = (backgroundColor.getRGB4() << 4) | foregroundColor.getRGB4();

    if (c == '\n') {
        cgaMemory.setByte(' ', position);
        cgaMemory.setByte(0, position + 1);
        currentRow++;
        currentColumn = 0;
    } else {
        cgaMemory.setByte(c, position);
        cgaMemory.setByte(colorAttribute, position + 1);
        currentColumn++;
    }

    if (currentColumn >= getColumns()) {
        currentRow++;
        currentColumn = 0;
    }

    if (currentRow >= getRows()) {
        scrollUp();
        currentColumn = 0;
        currentRow = getRows() - 1;
    }

    updateCursorPosition();
}

void ColorGraphicsArray::clear(const Util::Graphic::Color &backgroundColor) {
    for (uint32_t i = 0; i < getRows() * getColumns(); i++) {
        cgaMemory.setShort(0x000f);
    }

    currentRow = 0;
    currentColumn = 0;
    updateCursorPosition();
}

void ColorGraphicsArray::setPosition(uint16_t column, uint16_t row) {
    currentColumn = column;
    currentRow = row;
}

void ColorGraphicsArray::updateCursorPosition() {
    uint16_t position = currentRow * getColumns() + currentColumn;
    auto low  = static_cast<uint8_t>(position & 0xff);
    auto high = static_cast<uint8_t>((position >> 8) & 0xff);

    // Write high byte
    indexPort.writeByte(CURSOR_HIGH_BYTE);
    dataPort.writeByte(high);

    // Write low byte
    indexPort.writeByte(CURSOR_LOW_BYTE);
    dataPort.writeByte(low);
}

void ColorGraphicsArray::scrollUp() {
    auto columns = getColumns();
    auto rows = getRows();

    // Move screen upwards by one row
    auto source = cgaMemory.add(columns * BYTES_PER_CHARACTER);
    cgaMemory.copyRange(source, columns * (rows - 1) * BYTES_PER_CHARACTER);

    // Clear last row
    auto clear = cgaMemory.add(columns * (rows - 1) * BYTES_PER_CHARACTER);
    for (uint32_t i = 0; i < getColumns(); i++) {
        clear.add(i * 2).setShort(0x000f);
    }
}

ColorGraphicsArray::~ColorGraphicsArray() {
    delete reinterpret_cast<uint8_t*>(cgaMemory.get());
}

}