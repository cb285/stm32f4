#include "log.h"

uint32_t Log2(uint32_t x) {
    return (sizeof(uint32_t) * 8) - __builtin_clz(x) - 1;
}
