#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Panic_Context is used to save the CPU state when a panic occurs
 * and recover it when the panic is handled. It is architecture-specific and
 * must be implemented for each supported architecture.
 */

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
_Noreturn void _panic_context_restore(struct Panic_Context* context);

/* The Panic_Frame describes a frame in the panic handling stack.
 * Catch kind is used for frames that catch a downstream panic and handle it.
 * Guard kind is used for frames that guard a section of code and will rethrow
 * the panic if it occurs. Root kind is used for the root frame of the panic
 * handling stack.
 */

struct Panic_Info;

enum Panic_Frame_Kind
{
  Panic_Frame_Kind_Root,
  Panic_Frame_Kind_Guard,
  Panic_Frame_Kind_Catch,
};

typedef bool (*Panic_Frame_Predicate_Fn)(struct Panic_Info const* info);

struct Panic_Frame
{
  enum Panic_Frame_Kind kind;
  struct Panic_Frame* previous;
  struct Panic_Context context;
  Panic_Frame_Predicate_Fn predicate;
};

void _panic_frame_push(struct Panic_Frame* frame);
void _panic_frame_pop(void);

/* Panic_Info describes the information associated with a panic. */

typedef void (*Panic_Handle_Fn)(const struct Panic_Info* info);

struct Panic_Info
{
  size_t type;
  uint8_t* data;
  Panic_Handle_Fn handle;
};

#define PANIC_INFO_UNINIT {0, NULL, NULL}

_Noreturn void _panic_raise(struct Panic_Info info);