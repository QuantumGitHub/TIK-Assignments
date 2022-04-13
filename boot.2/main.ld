PROVIDE(uart = 0x10000000);

SECTIONS
{
	. = 0x80000000;
	.text : { stack.o(.text) *(.text) }
	.data : { *(.data) }
	.bss : { *(.bss) *(COMMON) }
	. = . + SIZEOF(.bss);
	phys_base = ALIGN(0x1000);
}
