#ifndef __PIC_H
#define __PIC_H

#include <stdint.h>

void pic_install(void);
void pic_disable(void);
void pic_eoi(uint8_t no);

#endif