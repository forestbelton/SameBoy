#include "gb.h"
#include <Windows.h>

struct GB_hook_s {
    uint16_t addr;
    uint16_t bank; /* -1 = any bank*/
};

void GB_hooks_init(GB_gameboy_t *gb) {
}

unsigned GB_hooks_run(GB_gameboy_t *gb) {
}
