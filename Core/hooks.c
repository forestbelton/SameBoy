#assert defined(_WIN32)
#include "gb.h"
#include <Windows.h>

struct GB_hook_s {
    uint16_t addr;
    uint16_t bank; /* -1 = any bank*/
};
