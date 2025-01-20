## Handling the Shift key [20 points]

The keyboard driver in
[keyboard.c](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/drivers/keyboard.c)
does not support the Shift key correctly. When the Shift key is pressed, a
question mark ('?') appears on the screen.

To resolve this, you need to determine the scancodes sent by the keyboard when
the Shift keys are pressed and released. Start a debug session with `make
debug`, set a breakpoint at the `interrupt_handler` function, and press the
Shift keys in the Qemu window. Make sure to test both the left and right Shift
keys.

Next, implement support for the Shift keys in the keyboard driver. When either
Shift key is pressed, letter keys should produce uppercase letters (e.g., `A`
when typing Shift+A), and digit keys should produce special symbols (e.g., `%`
when typing Shift+5).

You will need to add static variables to `keyboard.c` to keep track of the state
of the Shift keys.

## Handling the Backspace key [20 points]

Yabloko does not handle Backspace correctly. If you press it, a question mark
appears on the screen. Your task is to fix this.

Start by implementing a `vga_backspace` function in
[vga.c](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/drivers/vga.c).
The function should move the cursor one position back with `vga_set_cursor`
and output a space in this position, so
```
> some texts_
```
(with the cursor as `_`) becomes
```
> some text_
```

If the cursor is already at the beginning of the screen (`vga_get_cursor`
returns 0), `vga_backspace` should do nothing.

As your next step, figure out the scancode of Backspace (see previous task) and
process it in `interrupt_handler` appropriately (decreasing `kbd_buf_size` and
calling `vga_backspace` if `kbd_buf_size` was nonzero).

## Drawing a spinner [20 points]
The programmable interrupt timer (PIT) of the simulated computer
is configured to generate an interrupt approximately 100 times per second.
You can arrange for your handler to be called every time this happens
with `add_timer_callback(my_handler)` somewhere inside `kmain`.

Your task is to draw a white-on-black spinner in the upper right corner of the screen,
rotating once per second. The spinner can be drawn by displaying the next symbol
in the sequence `↑/→\↓/←\`. The arrows have codes 0x18-0x1b in the encoding
used by the simulated terminal ([CP437](https://en.wikipedia.org/wiki/Code_page_437)).

You will need to make the `vga_set_char` function
in `vga.c` more configurable, so that it can output symbols of different colors.

## Support `.bss` [60 points]
Yabloko does not honor the size of `.bss` that your program requests.
Instead, it [reads your program file](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/proc.c#L53)
into memory, reserving just enough pages that it fits. If you create a large
`.bss` with (for example) `static char zeroes[8000]`, then accessing `zeroes[7000]` will
cause a page fault (try running the program in `user/bss.c` by typing `run bss` at
the Yabloko prompt).

Fix this by parsing the header of the executable file and mapping the required amount
of pages. We already have the header structure `hdr` in [proc.c](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/proc.c#L58).

There are `hdr->e_phnum` program sections in the executable. The first program header
starts `hdr->e_phoff` bytes after `hdr`, and each header has size `hdr->e_phentsize`.

Treat each program header as a `Elf32_Phdr` structure (see [`man elf`](https://man7.org/linux/man-pages/man5/elf.5.html)).
The corresponding program section starts at `phdr->p_vaddr` and has size
`phdr->p_memsz` in memory. Figure out the highest page that any of the sections
reaches into, and map the required pages with a call to `allocuvm`,
similar to how they were [mapped for the executable](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/proc.c#L49).
The pages mapped by `allocuvm` are automatically zeroed out.

## Checking user supplied pointers [100 points]

This task requires understanding of virtual memory mappings and the memory
layout of Yabloko. Contact [@myltsev](https://t.me/myltsev) if you attempt
this and get stuck.

The `badputs` executable [demonstrates](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/user/badputs.c#L14)
the use of the `SYS_puts` system call. It supplies the string pointer as an argument
of the system call, and the string is printed to the screen.

However, the kernel does not actually check if the string pointer is correct!
It is easy to cause a panic by supplying a random value:
```
    syscall(SYS_puts, 0x12345678);
    // Kernel panic: Page Fault
```

Your task is to check if the pointer actually points to a valid zero-terminated string,
completing a [FIXME](https://gitlab.myltsev.ru/caos/hw/yabloko/-/blob/main/cpu/idt.c#L124)
in the code. As a result, `run badputs` should no longer cause a kernel panic.

### Figuring out the size of mapped memory after the pointer
During the handling of this system call, the page table of the calling process
is installed. Our first task is to figure out if the virtual address in the pointer
is mapped to a page readable by userspace, and how many bytes after this address
are also readable by userspace.

Let us consider an example. Let's say that in the curent page table, two pages
are mapped at 0x1234000 and 0x1235000, both readable by userspace, and no
mapping exists at 0x1236000. If the user supplies the pointer 0x1234ff0, then
0x100f bytes are readable after it.

The current page directory is accessible in `proc.c` as `vm.user_task->pgdir`.
This is a virtual address of the page directory, so you can read the first entry
of the page directory as `pde_t entry = vm.user_task->pgdir[0]`.

The page directory holds *physical* addresses of second level page tables.
If an entry of the page directory is present (`entry & PTE_P`), then
you can retrieve a virtual address of the page table as
`pte_t *table = P2V(PTE_ADDR(entry))`.

In order to be readable by userspace, a page table entry has to be present
and user accessible (both `PTE_P` and `PTE_U` have to be set).

Write a function `uintptr_t user_readable_after(uintptr_t ptr)` which
calculates the required value (e.g., 0 for a pointer that does not belong
to a mapped page).

### Checking that a zero-terminated string exists at the pointer
Now we just have to check that there is a null terminator
somewhere inside the user-readable memory after the pointer.
You can directly read bytes with `((const char*)ptr)[index]`.
