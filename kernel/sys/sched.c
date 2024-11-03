#include <mm/pmm.h>
#include <cpu/tables/idt.h>
#include <dev/pit.h>
#include <dev/lapic.h>
#include <lib/libc.h>
#include <lib/alloc.h>
#include <lib/panic.h>
#include <lib/printf.h>
#include <sys/sched.h>

uint32_t sched_pid = 0;
uint32_t sched_quantum = 0;
struct task_t *sched_tasks[256];
struct task_t *current_task = NULL;

void sched_idle() {
    for (;;)
        asm volatile ("hlt");
}

struct task_t* sched_new_task(void* handler) {
    struct task_t* task = (struct task_t*)kmalloc(sizeof(struct task_t));

    task->pid = handler == sched_idle ? 0 : sched_pid++;
    task->quantum = 20;
    task->ctx.esp = (uint32_t)HIGHER_HALF(pmm_alloc(1));

    for (int i = 0; i < 256; i++) {
        if (sched_tasks[i] == NULL) {
            sched_tasks[i] = task;
            return task;
        }
    }
    return NULL;
}

struct task_t *sched_get_next_task() {
    for (int i = 0; i < 255; i++) {
        if (sched_tasks[i] == current_task && sched_tasks[i + 1] != NULL) {
            return sched_tasks[i + 1];
        }
    }
    return sched_tasks[0];
}

void sched_schedule(struct registers *r) {
    lapic_stop_timer();

    if (sched_quantum > 0) {
        sched_quantum--;
        lapic_eoi();
        lapic_oneshot(0x80, 5);
        return;
    }

    current_task = sched_get_next_task();
    sched_quantum = current_task->quantum;

    if (current_task)
        *r = current_task->ctx;

    lapic_eoi();
    lapic_oneshot(0x80, 5);
}

void sched_install() {
    memset(sched_tasks, 0x00, sizeof(sched_tasks));

    sched_new_task(sched_idle);
    irq_register(0x80 - 32, sched_schedule);

    printf("[%5d.%04d] %s:%d: initialized scheduler\n", pit_ticks / 10000, pit_ticks % 10000, __FILE__, __LINE__);
}