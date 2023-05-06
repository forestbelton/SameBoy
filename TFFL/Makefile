DLL := ../build/bin/SDL/gb_hooks.dll

.PHONY: clean

CFILES := tffl.c
OFILES := $(CFILES:%.c=%.o)

CFLAGS := -std=gnu11 -I../Core -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS := -shared

$(DLL): $(OFILES)
	clang $(LDFLAGS) -o $@ $^

%.o: %.c
	clang $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(DLL) $(OFILES)
