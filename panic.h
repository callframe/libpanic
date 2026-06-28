#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct Panic_Info;

typedef void (*Panic_Handle_Fn)(const struct Panic_Info* info);

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

#define _panic_context_uninit (struct Panic_Context){0}

__attribute__((returns_twice)) bool _panic_context_save(
    struct Panic_Context* context);
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

extern _Thread_local struct Panic_Frame* _PANIC_FRAME;

#define _panic_frame_init(__kind, __predicate) \
  (struct Panic_Frame) { __kind, NULL, _panic_context_uninit, __predicate }

void _panic_frame_push(struct Panic_Frame* frame);
void _panic_frame_pop(void);

/* Panic_Info describes the information associated with a panic. */

struct Panic_Info
{
  size_t type;
  uint8_t* data;

  char const* file;
  uint32_t line;
};

#define _panic_info_init(__type, __data) \
  {__type, (uint8_t*)__data, __FILE__, __LINE__}

#define _panic_info_uninit _panic_info_init(0, NULL)

_Noreturn void _panic_raise(struct Panic_Info info);

/* Try/Catch API */

#define _panic_try_frame(__predicate) \
  _panic_frame_init(Panic_Frame_Kind_Catch, (__predicate))

#define _panic_try_begin(__frame) \
  (_panic_frame_push(&(__frame)), !_panic_context_save(&(__frame).context))

#define _panic_try_end(__once) (_panic_frame_pop(), (__once) = NULL)
#define _panic_catch_begin() _panic_frame_pop()
#define _panic_catch_end(__once) ((__once) = NULL)

#define panic_try(__predicate)                                            \
  for (struct Panic_Frame __panic__frame = _panic_try_frame(__predicate), \
                          *__panic__once = &__panic__frame;               \
       __panic__once != NULL; __panic__once = NULL)                       \
    if (_panic_try_begin(__panic__frame))                                 \
      for (; __panic__once != NULL; _panic_try_end(__panic__once))

#define panic_catch                                      \
  else for (_panic_catch_begin(); __panic__once != NULL; \
            _panic_catch_end(__panic__once))

/* Panic API */
#define panic_raise(__type, __data) \
  _panic_raise((struct Panic_Info)_panic_info_init((__type), (__data)))

#define panic_return(__value) \
  do                          \
  {                           \
    _panic_frame_pop();       \
    return (__value);         \
  } while (0)

#define panic_return_void() \
  do                        \
  {                         \
    _panic_frame_pop();     \
    return;                 \
  } while (0)
