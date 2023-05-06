#include "gb.h"
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
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

const char *now() {
    static struct tm tm;
    static time_t now;
    static char buf[sizeof "2011-10-08T07:07:09Z"];
    time(&now);
    gmtime_s(&tm, &now);
    strftime(&buf[0], sizeof buf, "%FT%TZ", &tm);
    return &buf[0];
}

void tlogf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(LOG, "%s - ", now());
    vfprintf(LOG, fmt, args);
    fprintf(LOG, "\n");
    va_end(args);
}

__declspec(dllexport) struct GB_hooklist_s *GetHookList() {
    fopen_s(&LOG, "tffl.log", "a+");
    tlogf("Logger initialized");
    return &HOOKLIST;
}

unsigned hook_entry(GB_gameboy_t *gb) {
    tlogf("Entry point hooked");
    return 0;
}
