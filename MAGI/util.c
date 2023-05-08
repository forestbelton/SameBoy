#include "magi.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#define LOGFILE_PATH "magi.log"

static FILE *LOG;

const char *now() {
    static char buf[sizeof "2011-10-08T07:07:09Z"];
    time_t now;
    struct tm tm;
    time(&now);
    gmtime_s(&tm, &now);
    strftime(&buf[0], sizeof buf, "%FT%TZ", &tm);
    return &buf[0];
}

void mlogf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    if (LOG == NULL) {
        // TODO: Error checking
        fopen_s(&LOG, LOGFILE_PATH, "w");
        if (LOG == NULL) {
            return;
        }
    }
    fprintf(LOG, "%s - ", now());
    vfprintf(LOG, fmt, args);
    fprintf(LOG, "\n");
    va_end(args);
}