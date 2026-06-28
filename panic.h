#pragma once

#include <stdbool.h>
#include <stdint.h>

struct Panic_Context
{
#if defined(__x86_64__)
  int64_t rbx;
  int64_t rbp;
  int64_t rsp;
  int64_t r12;
  int64_t r13;
  int64_t r14;
  int64_t r15;
  int64_t rip;

#elif defined(__i386__)
  int32_t ebx;
  int32_t ebp;
  int32_t esp;
  int32_t esi;
  int32_t edi;
  int32_t eip;

#else
#error "Target CPU architecture is not supported"
#endif
};

bool _panic_context_save(struct Panic_Context* context);
void _panic_context_restore(struct Panic_Context* context);