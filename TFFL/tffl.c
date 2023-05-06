#include "gb.h"
#include <stdio.h>
#include <Windows.h>

FILE *LOG;

unsigned hook_entry(GB_gameboy_t *);

struct GB_hook_s HOOKS[1] = {
    { .addr = 0x100, .bank = 1, .fn = &hook_entry },
};

struct GB_hooklist_s HOOKLIST = {
    .n_hooks = (sizeof HOOKS / sizeof HOOKS[0]),
    .entry = &HOOKS[0]
};

__declspec(dllexport) struct GB_hooklist_s *GetHookList() {
    fopen_s(&LOG, "tffl.log", "a+");
    fprintf(LOG, "[HOOK-DLL] Logger initialized\n");
    return &HOOKLIST;
}

unsigned hook_entry(GB_gameboy_t *gb) {
    fprintf(LOG, "[HOOK-DLL] Entry point hooked\n");
    return 0;
}
