#include "elf.h"
#include "proc.h"
#include "fs/fs.h"
#include "cpu/gdt.h"
#include "cpu/isr.h"
#include "lib/mem.h"
#include "lib/string.h"
#include "console.h"

struct context {
    // matches the behavior of swtch()
	uint32_t edi, esi, ebp, ebx;
	uint32_t eip; // return address for swtch()
};

struct kstack {
    uint32_t space[400];
    struct context context;
    registers_t trapframe;
};

struct vm {
    void *kernel_thread;
    void *user_thread;
    struct kstack *user_kstack;
} *vm;

void trapret();
void swtch(void** oldstack, void* newstack);

void run_elf(const char* name) {
    if (!vm) {
        vm = kmalloc(sizeof(struct vm));
        vm->user_kstack = kmalloc(sizeof(struct kstack));
    }
    if (read_file(name, (void*)USER_BASE, 100 << 20) <= 0) {
        printk(name);
        printk(": file not found\n");
        return;
    }
    Elf32_Ehdr *hdr = (void*)USER_BASE;

    struct kstack *u = vm->user_kstack;
    memset(u, 0, sizeof(*u));
    u->context.eip = (uint32_t)trapret;

    registers_t *tf = &u->trapframe;
    tf->eip = hdr->e_entry;
    tf->cs = (SEG_UCODE << 3) | DPL_USER;
    tf->ds = (SEG_UDATA << 3) | DPL_USER;
    tf->es = tf->ds;
    tf->fs = tf->ds;
    tf->gs = tf->ds;
    tf->ss = tf->ds;
    tf->eflags = FL_IF;
    tf->useresp = USER_STACK_BASE;

    // initialization done, now switch to the process
    swtch(&vm->kernel_thread, &u->context);

    // process has finished
}
