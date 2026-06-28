.section .text, "ax", @progbits

/* bool _panic_context_save(struct Panic_Context* context); */

.globl _panic_context_save
.type _panic_context_save, @function
.p2align 4

/* Saves the current context into the given Panic_Context structure. */
#if defined(__x86_64__)

/*
  int64_t rbx;
  int64_t rbp;
  int64_t rsp;
  int64_t r12;
  int64_t r13;
  int64_t r14;
  int64_t r15;
  int64_t rip;
*/

_panic_context_save:
/* Save all callee-saved registers */
    movq %rbx, 0(%rdi)
    movq %rbp, 8(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 32(%rdi)
    movq %r14, 40(%rdi)
    movq %r15, 48(%rdi)

/* Save the return address */
    movq (%rsp), %rax
    movq %rax, 56(%rdi)    
    leaq 8(%rsp), %rax
    movq %rax, 16(%rdi)

/* Return 0 to indicate that the context was saved successfully */
    xorb %al, %al
    ret

#elif defined(__i386__)

/*
  int32_t ebx;
  int32_t ebp;
  int32_t esp;
  int32_t esi;
  int32_t edi;
  int32_t eip;
*/

_panic_context_save:
/* Load the context pointer from the stack */
    movl 4(%esp), %eax

/* Save all callee-saved registers */
    movl %ebx, 0(%eax)
    movl %ebp, 4(%eax)
    movl %esi, 12(%eax)
    movl %edi, 16(%eax)

/* Save the return address */
    movl (%esp), %ecx
    movl %ecx, 20(%eax)
    leal 4(%esp), %ecx
    movl %ecx, 8(%eax)

/* Return 0 to indicate that the context was saved successfully */
    xorb %al, %al
    ret

#else
#error "Target architecture not supported"
#endif

/* void _panic_context_restore(struct Panic_Context* context); */

.globl _panic_context_restore
.type _panic_context_restore, @function
.p2align 4

/* Restores the context from the given Panic_Context structure. */
#if defined(__x86_64__)

/*
  int64_t rbx;
  int64_t rbp;
  int64_t rsp;
  int64_t r12;
  int64_t r13;
  int64_t r14;
  int64_t r15;
  int64_t rip;
*/

_panic_context_restore:
/* Restore all callee-saved registers */
    movq 0(%rdi), %rbx
    movq 8(%rdi), %rbp
    movq 24(%rdi), %r12
    movq 32(%rdi), %r13
    movq 40(%rdi), %r14
    movq 48(%rdi), %r15

/* Load the return address into a scratch register, then restore the stack */
    movq 56(%rdi), %rdx
    movq 16(%rdi), %rsp

/* Return 1 to indicate that the context was restored, then jump back */
    movl $1, %eax
    jmp *%rdx

#elif defined(__i386__)

/*
  int32_t ebx;
  int32_t ebp;
  int32_t esp;
  int32_t esi;
  int32_t edi;
  int32_t eip;
*/

_panic_context_restore:
/* Load the context pointer from the stack */
    movl 4(%esp), %eax

/* Restore all callee-saved registers */
    movl 0(%eax),  %ebx
    movl 4(%eax),  %ebp
    movl 12(%eax), %esi
    movl 16(%eax), %edi

/* Load the return address into a scratch register, then restore the stack */
    movl 20(%eax), %edx
    movl 8(%eax),  %esp

/* Return 1 to indicate that the context was restored, then jump back */
    movl $1, %eax
    jmp *%edx

#else
#error "Target architecture not supported"
#endif

.section .note.GNU-stack,"",@progbits