SECTIONS
{
.text (0x1000) : {
 startup.o( .text )
}

.data : ALIGN(CONSTANT(MAXPAGESIZE)) {
_DATA_START_ = .;
 *(.data)
_DATA_END_ = .;
}

.rodata : ALIGN(CONSTANT(MAXPAGESIZE)) {
_RODATA_START_ = .;
 *(.rodata)
_RODATA_START_ = .;
}

.bss : ALIGN(CONSTANT(MAXPAGESIZE)) {
_BSS_START_ = .;
 *(.bss)
_BSS_END_ = .;
}
}