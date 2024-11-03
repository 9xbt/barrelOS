OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_main)

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    . = 0xffffffff80000000;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    text_start_ld = .;

    .text : {
        *(.text .text.*)
    } :text

    text_end_ld = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    rodata_start_ld = .;

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    rodata_end_ld = .;

    . = ALIGN(CONSTANT(MAXPAGESIZE));

    data_start_ld = .;

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    data_end_ld = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.note .note.*)
    }
}