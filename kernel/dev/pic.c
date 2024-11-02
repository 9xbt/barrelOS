#include <cpu/io.h>
#include <dev/pit.h>
#include <lib/printf.h>

#define PIC1            0x20        /* IO base address for master PIC */
#define PIC2            0x20        /* IO base address for slave PIC */
#define PIC1_CMD        PIC1
#define PIC1_DAT        (PIC1+1)
#define PIC2_CMD        PIC2
#define PIC2_DAT        (PIC2+1)

#define ICW1_ICW4       0x01        /* Indicates that ICW4 will be present */
#define ICW1_SINGLE     0x02        /* Single (cascade) mode */
#define ICW1_INTERLVAL4 0x04        /* Call address interval 4 (8) */
#define ICW1_LEVEL      0x08        /* Level triggered (edge) mode */
#define ICW1_INIT       0x10        /* Initialization */

#define ICW4_8086       0x01        /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO       0x02        /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE  0x08        /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C        /* Buffered mode/master */
#define ICW4_SFNM       0x10        /* Special fully nested (not) */

#define PIC_EOI         0x20        /* End of interrupt */

void pic_install(void) {
    outb(PIC1_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_CMD, ICW1_INIT | ICW1_ICW4);
    outb(PIC1_DAT, 0x20);
    outb(PIC2_DAT, 0x28);
    outb(PIC1_DAT, 0x04);
    outb(PIC2_DAT, 0x02);
    outb(PIC1_DAT, ICW4_8086);
    outb(PIC2_DAT, ICW4_8086);

    outb(PIC1_DAT, 0x00);
    outb(PIC2_DAT, 0x00);

    asm volatile ("sti");
    printf("[%5d.%04d] %s:%d: initialized 8259 PIC and enabled interrupts\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}

void pic_disable(void) {
    outb(PIC1_DAT, 0xFF);
    outb(PIC2_DAT, 0xFF);
    outb(PIC1_CMD, 0x20);
    outb(PIC2_CMD, 0x20);

    printf("[%5d.%04d] %s:%d: disabled 8259 PIC\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}

void pic_eoi(uint8_t no) {
    if (no >= 8)
        outb(PIC2, PIC_EOI);
    outb(PIC1, PIC_EOI);
}