SECTIONS
{
.text (0x1000) : {
 *( .text )
}

.rodata : {
_RODATA_START_ = .;
 *(.rodata)
_RODATA_START_ = .;
}

.data : ALIGN(CONSTANT(MAXPAGESIZE)) {
_DATA_START_ = .;
 *(.data)
_DATA_END_ = .;
}


.bss : ALIGN(CONSTANT(MAXPAGESIZE)) {
_BSS_START_ = .;
 *(.bss)
_BSS_END_ = .;
}
}
