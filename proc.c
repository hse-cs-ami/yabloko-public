#include "elf.h"
#include "proc.h"
#include "fs/fs.h"
#include "cpu/gdt.h"
#include "cpu/isr.h"
#include "cpu/memlayout.h"
#include "kernel/mem.h"
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
    char bottom[];
};

struct task {
    struct taskstate tss;
    pde_t *pgdir;
    struct kstack stack;
};

struct vm {
    void *kernel_thread;
    struct task *user_task;
} vm;

void trapret();
void swtch(void** oldstack, void* newstack);

void run_elf(const char* name) {
    struct stat statbuf;
    if (stat(name, &statbuf) != 0) {
        printk(name);
        printk(": file not found\n");
        return;
    }
    if (!vm.user_task) {
        vm.user_task = kalloc();
    }
    vm.user_task->pgdir = setupkvm();
    allocuvm(vm.user_task->pgdir, USER_BASE, USER_BASE + statbuf.size);
    allocuvm(vm.user_task->pgdir, USER_STACK_BASE - 2 * PGSIZE, USER_STACK_BASE);
    switchuvm(&vm.user_task->tss, vm.user_task->stack.bottom, vm.user_task->pgdir);

    if (read_file(&statbuf, (void*)USER_BASE, 100 << 20) <= 0) {
        printk(name);
        printk(": file not found\n");
        return;
    }
    Elf32_Ehdr *hdr = (void*)USER_BASE;

    struct kstack *u = &vm.user_task->stack;
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
    swtch(&vm.kernel_thread, &u->context);

    // process has finished
}

_Noreturn void killproc() {
    void* task_stack;
    switchkvm();
    freevm(vm.user_task->pgdir);
    sti();
    swtch(&task_stack, vm.kernel_thread);
    __builtin_unreachable();
}
