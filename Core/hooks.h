#ifndef hooks_h
#define hooks_h

#include "gb.h"

// TODO: Expand to multiple platforms with dlopen and such
#define HOOKS_MODULE_NAME "gb_hooks.dll"
#define HOOK_MAX_NAME_LENGTH 16

typedef unsigned (*GB_hook_fn_t)(GB_gameboy_t *gb);

struct GB_hook_s {
    uint16_t addr;
    uint16_t bank; /* -1 = any bank*/
    char name[HOOK_MAX_NAME_LENGTH + 1];
    GB_hook_fn_t fn;
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
