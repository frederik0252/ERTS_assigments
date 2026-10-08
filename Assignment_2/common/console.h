#ifndef ERTS_CONSOLE_H
#define ERTS_CONSOLE_H

#include <stddef.h>
#include "xil_printf.h"

static inline void Console_ReadLine(char *buffer, size_t capacity)
{
    size_t used = 0U;

    if (capacity == 0U) {
        return;
    }
    for (;;) {
        const char ch = inbyte();
        if (ch == '\r' || ch == '\n') {
            if (used == 0U) {
                continue;
            }
            outbyte('\r');
            outbyte('\n');
            break;
        }
        if ((ch == '\b' || ch == 127) && used > 0U) {
            --used;
            xil_printf("\b \b");
        } else if (ch >= 32 && ch <= 126 && used + 1U < capacity) {
            buffer[used++] = ch;
            outbyte(ch);
        }
    }
    buffer[used] = '\0';
}

#endif

