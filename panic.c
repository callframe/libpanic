#include "panic.h"

extern Panic_Handle_Fn _PANIC_HANDLER;

_Thread_local struct Panic_Frame* _PANIC_FRAME = NULL;
_Thread_local struct Panic_Info _PANIC_INFO = _panic_info_uninit;

void _panic_frame_push(struct Panic_Frame* frame)
{
  frame->previous = _PANIC_FRAME;
  _PANIC_FRAME = frame;
}

void _panic_frame_pop(void) { _PANIC_FRAME = _PANIC_FRAME->previous; }

_Noreturn void _panic_raise(struct Panic_Info info)
{
  _PANIC_INFO = info;

  struct Panic_Frame* frame = _PANIC_FRAME;
  while (frame != NULL)
  {
    switch (frame->kind)
    {
      case Panic_Frame_Kind_Guard:
        _PANIC_FRAME = frame;
        _panic_context_restore(&frame->context);  // Does not return
        __builtin_unreachable();

      case Panic_Frame_Kind_Catch:
        if (frame->predicate == NULL || frame->predicate(&_PANIC_INFO))
        {
          _PANIC_FRAME = frame;
          _panic_context_restore(&frame->context);  // Does not return
        }
        break;  // If predicate doesn't match we continue unwinding the stack

      case Panic_Frame_Kind_Root:
        goto fail;
    }

    frame = frame->previous;
  }

fail:
  _PANIC_HANDLER(&_PANIC_INFO);
  __builtin_unreachable();
}