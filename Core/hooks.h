#ifndef hooks_h
#define hooks_h

#include "gb.h"

// TODO: Expand to multiple platforms with dlopen and such
#define HOOKS_MODULE_NAME "gb_hooks.dll"

struct GB_hook_s {
    uint16_t addr;
    uint16_t bank; /* -1 = any bank*/
    unsigned (*fn)(GB_gameboy_t *);
};

struct GB_hooklist_s {
    uint16_t n_hooks;
    struct GB_hook_s *entry;
};

typedef struct GB_hooklist_s *(*GB_hooklist_getter_t)(void);

#ifdef GB_INTERNAL
void GB_hooks_init(GB_gameboy_t *gb);
unsigned GB_hooks_run(GB_gameboy_t *gb);
#endif

#endif
