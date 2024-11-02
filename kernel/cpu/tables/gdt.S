section .text

[extern gdt_descriptor]
[global gdt_flush]

gdt_flush:
    lgdt [gdt_descriptor]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.ret
gdt_flush.ret:
    ret