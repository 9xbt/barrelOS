#ifndef __SCHED_H
#define __SCHED_H

#include <cpu/tables/idt.h>

struct task_t {
    struct registers ctx;

    uint32_t pid;
    uint32_t quantum;
};

void sched_install();

#endif