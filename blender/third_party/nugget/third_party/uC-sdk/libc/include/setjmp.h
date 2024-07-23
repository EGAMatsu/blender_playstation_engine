#pragma once

#include <decl.h>
#include <stdint.h>

typedef struct {
    uint32_t buf[32];
} jmp_buf;

BEGIN_DECL

int setjmp(jmp_buf env);
void longjmp(jmp_buf env, int val);

END_DECL
