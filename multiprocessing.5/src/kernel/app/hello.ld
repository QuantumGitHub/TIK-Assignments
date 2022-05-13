PROVIDE(uart = 0x10000000);

SECTIONS
{
	. = 0x80000000;
	.text : { *(.text) }
	.rodata : { *(.rodata) }
}
