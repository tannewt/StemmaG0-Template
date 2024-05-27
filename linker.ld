flash_base = 0x08000000;
flash_size = 32K;
ram_base = 0x20000000;
ram_size = 8K;
ENTRY(main)
MEMORY
{
    FLASH (rx)     : ORIGIN = flash_base, LENGTH = flash_size
    RAM  (rwx)     : ORIGIN = ram_base, LENGTH = ram_size
}

SECTIONS
{
    .text :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        *(.text*)                /* .text* sections (code) */

        . = ALIGN(4);
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
        *(.gnu.linkonce.armexidx.*)
        _etext = .;        /* define a global symbol at end of code */
        __etext = .;       /* define a global symbol at end of code */
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        *(.data*)               /* .data* sections */
        . = ALIGN(4);
    } > RAM AT> FLASH
    _sdata = ADDR(.data);
    _sidata = LOADADDR(.data);
    _edata = _sdata + SIZEOF(.data);

    /* Uninitialized data section */
    .bss :
    {
        . = ALIGN(4);

        *(.bss*)
        *(COMMON)

        . = ALIGN(4);
    } > RAM
    _sbss = ADDR(.bss);
    _ebss = _sbss + SIZEOF(.bss);

    _estack = ORIGIN(RAM) + LENGTH(RAM);
}