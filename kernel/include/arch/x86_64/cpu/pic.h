#ifndef CPU_PIC_H
#define CPU_PIC_H

#include <stdint.h>

void pic_send_eoi(uint8_t irq);
void pic_remap(uint8_t offset1, uint8_t offset2);
void pic_disable(void);
void pic_set_mask(uint8_t irq_line);
void pic_clear_mask(uint8_t irq_line);

#endif // CPU_PIC_H