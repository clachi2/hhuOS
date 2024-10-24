#ifndef HELPER_H
#define HELPER_H

#include <cstdint>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"
#define print(i) Util::System::out << i << Util::Io::PrintStream::endl << Util::Io::PrintStream::flush
#pragma clang diagnostic pop

uint32_t blendPixels(uint32_t lower, uint32_t upper);

void blendBuffers(uint32_t *lower, const uint32_t *upper, int size);

void blendBuffers(uint32_t *lower, const uint32_t *upper, int lx, int ly, int ux, int uy, int px, int py);

const char *int_to_string(int value);

const char *double_to_string(double value, int decimal_places);

int min(int a, int b);

int max(int a, int b);

namespace Bitmaps {

    extern uint8_t arrow_up[];
    extern uint8_t arrow_down[];
    extern uint8_t trashcan[];
    extern uint8_t eye[];
    extern uint8_t arrow_back[];
    extern uint8_t cross[];
    extern uint8_t checkmark[];

} // namespace Bitmaps

#endif // HELPER_H

