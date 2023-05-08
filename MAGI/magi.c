#include "magi.h"
#include "gb.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <Windows.h>

typedef uint8_t (*gb_memory_reader_t)(GB_gameboy_t *gb, uint16_t addr);
typedef void (*gb_memory_writer_t)(GB_gameboy_t *gb, uint16_t addr, uint8_t value);

boolean inited;
gb_memory_reader_t gb_read8;
gb_memory_writer_t gb_write8;

// 16-bit ops are in little-endian
static uint16_t gb_read16(GB_gameboy_t *gb, uint16_t addr) {
    return (gb_read8(gb, addr + 1) << 8) | gb_read8(gb, addr);
}

static void gb_write16(GB_gameboy_t *gb, uint16_t addr, uint16_t value) {
    gb_write8(gb, addr, value >> 8);
    gb_write8(gb, addr + 1, value & 0xff);
}

static bool _init() {
    if (inited) {
        return true;
    }
    const HMODULE emulator = GetModuleHandle(NULL);
    if (emulator == NULL) {
        mlogf("Could not get handle to emulator");
        return false;
    }
    gb_write8 = (gb_memory_writer_t)GetProcAddress(emulator, "GB_write_memory");
    if (gb_write8 == NULL) {
        mlogf("Could not find pointer to GB_write_memory");
        return false;
    }
    gb_read8 = (gb_memory_reader_t)GetProcAddress(emulator, "GB_read_memory");
    if (gb_read8 == NULL) {
        mlogf("Could not find pointer to GB_read_memory");
        return false;
    }
    inited = true;
    return true;
}

unsigned _mult8(GB_gameboy_t *gb) {
    // For simplicity, take mean number of cycles on uniform distribution of c
    const unsigned cycles = (4 + 12 + 8) + 7 * (8 + 8) + 8 * ((12 + 16) / 2);
    gb->hl = gb->b * gb->c;
    gb->pc = 0x0513;
    return cycles;
}

/*unsigned _gb_unsupported(GB_gameboy_t *gb) {
    return 0;
}*/

#define VBK 0xff4f

#define PENDING_TILE_DATA_PTR 0xc6e7
#define PENDING_TILE_VRAM_PTR 0xc6e9
#define PENDING_TILE_VRAM_BANK 0xc6eb

#define UNKNOWN_PTR_C6E4 0xc6e4
#define UNKNOWN_PTR_C6E5 0xc6e5

unsigned _vram_write_tile(GB_gameboy_t *gb) {
    uint16_t tile_vram_ptr = gb_read16(gb, PENDING_TILE_VRAM_PTR);
    uint16_t tile_data_ptr = gb_read16(gb, PENDING_TILE_DATA_PTR);

    const uint8_t tile_bank = gb_read8(gb, PENDING_TILE_VRAM_BANK);
    gb_write8(gb, VBK, tile_bank);

    mlogf("vram=$%04x, data=$%04x, bank=%u", tile_vram_ptr, tile_data_ptr, tile_bank);

    for (unsigned i = 0; i <= 0xf; ++i) {
        const uint8_t byte = gb_read8(gb, tile_data_ptr++);
        gb_write8(gb, tile_vram_ptr++, byte);
    }

    gb_write16(gb, PENDING_TILE_DATA_PTR, tile_data_ptr);
    gb_write16(gb, PENDING_TILE_VRAM_PTR, tile_vram_ptr);

    // Points to next byte after this function
    gb_write16(gb, 0xc6e4, 0x29a4);

    const unsigned cycles = (12 + 8 + 8 + 4) * 2 + 16 * 2 + (8 + 8 + 8) * 16 + 4 + 16 + 4 + 16 + 12 + 8 + 8 + 8 + 8 + 16 + 8 + 16;
    gb->pc = 0x29a3;

    return cycles;
}

// Writes an increasing sequence of B numbers at HL starting with the value in A
unsigned _memseq8(GB_gameboy_t *gb) {
    const unsigned cycles = (8 + 4 + 8 + 12) * gb->bc - 4;
    do {
        gb_write8(gb, gb->hl++, gb->a++);
        gb->b--;
    } while (gb->b > 0);
    gb->pc = 0x02e5;
    return cycles;
}

struct GB_hook_s HOOKS[] = {
    // Utilities
    { .addr = 0x04ca, .bank = 1, .fn = &_mult8, .name = "mult8" },
    //{ .addr = 0x0242, .bank = 1, .fn = &_gb_unsupported, .name = "gb_unsupported" },
    { .addr = 0x2949, .bank = 0xffff, .fn = &_vram_write_tile, .name = "vram_write_tile" },
    { .addr = 0x02e0, .bank = 6, .fn = &_memseq8, .name = "memseq8" },
};

struct GB_hooklist_s HOOKLIST = {
    .n_hooks = (sizeof HOOKS / sizeof HOOKS[0]),
    .entry = &HOOKS[0]
};

__declspec(dllexport) struct GB_hooklist_s *GetHookList() {
    if (!_init()) {
        return NULL;
    }
    return &HOOKLIST;
}
