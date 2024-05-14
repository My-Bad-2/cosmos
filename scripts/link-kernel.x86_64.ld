OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)

ENTRY(kmain)

PHDRS
{
    limine_requests PT_LOAD     FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    text            PT_LOAD     FLAGS((1 << 0) | (1 << 2)) ; /* Execute + Read */
    rodata          PT_LOAD     FLAGS((1 << 2)) ;            /* Read only */
    data            PT_LOAD     FLAGS((1 << 1) | (1 << 2)) ; /* Write + Read */
    dynamic         PT_DYNAMIC  FLAGS((1 << 1) | (1 << 2)) ; /* Dynamic PHDR for relocations */
}

SECTIONS
{
    . = 0xffffffff80000000;

    .limine_requests : {
        *(.limine_requests .limine_requests.*)
    } :limine_requests

    .limine_requests_delimiter : {
        *(.limine_requests_delimiter .limine_requests_delimiter.*)
    } :limine_requests

    . += CONSTANT(MAXPAGESIZE);

    .text : {
        *(.text .text.*)
    } :text

    . += CONSTANT(MAXPAGESIZE);

    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    .init_array : {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP(*(.init_array .ctors))
        PROVIDE_HIDDEN(__init_array_end = .);
    }

    . += CONSTANT(MAXPAGESIZE);

    .data : {
        *(.data .data.*)
    } :data

    .dynamic : {
        *(.dynamic)
    } :data :dynamic

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}