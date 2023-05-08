#include "gb.h"
#include <Windows.h>

#define GB_ASSERT(check, msg) do { \
    if (!(check)) { \
        GB_log(gb, "[HOOKS] ERROR: " msg "\n"); \
        return; \
    } \
} while(0)

void GB_hooks_init(GB_gameboy_t *gb) {
    GB_log(gb, "[HOOKS] Initializing...\n");
    gb->hooks = NULL;

    const HMODULE hooks_module = LoadLibrary("gb_hooks.dll");
    GB_ASSERT(hooks_module != NULL, "Could not find gb_hooks.dll");

    const GB_hooklist_getter_t getter = (GB_hooklist_getter_t)GetProcAddress(hooks_module, "GetHookList");
    GB_ASSERT(getter != NULL, "gb_hooks.dll does not export GetHookList");

    gb->hooks = getter();
    GB_ASSERT(gb->hooks != NULL, "GetHookList returned NULL");
    GB_log(gb, "[HOOKS] Loaded %d hooks.\n", gb->hooks->n_hooks);
}

int GB_hook_should_apply(GB_gameboy_t *gb, struct GB_hook_s *hook) {
    return gb->pc == hook->addr
        && (hook->bank == 0xffff || hook->bank == gb->mbc_rom_bank);
}

unsigned GB_hooks_run(GB_gameboy_t *gb) {
    if (!gb->boot_rom_finished || gb->hooks == NULL || gb->hooks->n_hooks == 0) {
        return 0;
    }
    // NB (forestbelton): We could do a binary search here like the
    //                    breakpoint code does, but I am lazy and
    //                    didn't want to mess around with unions and
    //                    sort keys. Switching to binary search may
    //                    become necessary later.
    struct GB_hook_s *entry = &gb->hooks->entry[0];
    for (uint16_t i = 0; i < gb->hooks->n_hooks; ++i) {
        if (!GB_hook_should_apply(gb, entry)) {
            entry++;
            continue;
        }
        GB_log(gb, "[HOOKS] Applying hook=%s;bank=$%02x;addr=%04x\n", entry->name, gb->mbc_rom_bank, gb->pc);
        return entry->fn(gb);
    }
    // No hooks ran!
    return 0;
}
