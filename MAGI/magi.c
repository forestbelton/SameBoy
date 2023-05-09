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

// MBC5
static void setRomBank(GB_gameboy_t *gb, uint8_t bank) {
    gb_write8(gb, ROM_BANK, bank);
    gb_write8(gb, 0x2000, bank);
}

// 16-bit ops are in little-endian
static uint16_t gb_read16(GB_gameboy_t *gb, uint16_t addr) {
    return (gb_read8(gb, addr + 1) << 8) | gb_read8(gb, addr);
}

static void gb_write16(GB_gameboy_t *gb, uint16_t addr, uint16_t value) {
    gb_write8(gb, addr + 1, value >> 8);
    gb_write8(gb, addr, value & 0xff);
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
    const unsigned cycles = (8 + 4 + 8 + 12) * gb->b - 4;
    do {
        gb_write8(gb, gb->hl++, gb->a++);
        gb->b--;
    } while (gb->b > 0);
    gb->pc = 0x02e5;
    return cycles;
}

static char STR_UNSUPPORTED[] =
    "Magi-Nation is   "
    "Specially Designed "
    "for Game Boy Color."
    "Please use a Game"
    " Boy Color To Play "
    " This Game.   ";

unsigned _gb_unsupported(GB_gameboy_t *gb) {
    uint16_t tile_vram_ptr = 0x8860;

    setRomBank(gb, 6);
    for (size_t i = 0; i < sizeof STR_UNSUPPORTED; ++i) {
        const uint16_t tile_data_ptr = 0x10 * STR_UNSUPPORTED[i] + 0x4000;
        gb_write8(gb, PENDING_TILE_VRAM_BANK, 1);
        gb_write16(gb, PENDING_TILE_DATA_PTR, tile_data_ptr);
        gb_write16(gb, PENDING_TILE_VRAM_PTR, tile_vram_ptr);
        mlogf("_gb_unsupported tile=$%04x vram=$%04x", tile_data_ptr, tile_vram_ptr);
        _vram_write_tile(gb);
        tile_vram_ptr += 0x10;
    }

    // NB: This is their way of writing tilemap data...
    #define MEMSEQ(av, hlv, bv) do { \
        gb->a = av; gb->b = bv; gb->hl = hlv; _memseq8(gb); \
    } while(0)
    MEMSEQ(0x86, 0x9c22, 0x10);
    MEMSEQ(0x97, 0x9c81, 0x12);
    MEMSEQ(0xa9, 0x9ce0, 0x14);
    MEMSEQ(0xbd, 0x9d44, 0xd);
    MEMSEQ(0xc9, 0x9da1, 0x13);
    MEMSEQ(0xdc, 0x9e01, 0x12);

    gb_write8(gb, IF, 0);
    gb_write8(gb, 0xff98, 0);
    gb_write8(gb, IE, 3);
    gb_write8(gb, LCDC, 0xe1);
    gb_write8(gb, IE, 0);

    return 0;
}

struct GB_hook_s HOOKS[] = {
    // Utilities
    { .addr = 0x04ca, .bank = 1, .fn = &_mult8, .name = "mult8" },
    { .addr = 0x0242, .bank = 1, .fn = &_gb_unsupported, .name = "gb_unsupported" },
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
