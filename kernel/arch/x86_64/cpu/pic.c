#include <arch.h>
#include <cpu/pic.h>
#include <log.h>

#define PIC1 0x20
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)

#define PIC2 0xA0
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20

#define ICW1_ICW4 0x01		 // Indicates that ICW4 will be present
#define ICW1_SINGLE 0x02	 // Single (cascade) mode
#define ICW1_INTERVAL4 0x04	 // Call address interval 4 (8)
#define ICW1_LEVEL 0x08		 // Level triggered (edge) mode
#define ICW1_INIT 0x10		 // Initialization - required!

#define ICW4_8086 0x01		  // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO 0x02		  // Auto (normal) EOI
#define ICW4_BUF_SLAVE 0x08	  // Buffered mode/slave
#define ICW4_BUF_MASTER 0x0C  // Buffered mode/master
#define ICW4_SFNM 0x10		  // Special fully nested (not)

void pic_send_eoi(uint8_t irq) {
	if (irq >= 8) {
		outpb(PIC2_COMMAND, PIC_EOI);
	}

	outpb(PIC1_COMMAND, PIC_EOI);
}

void pic_set_mask(uint8_t irq_line) {
    uint16_t port = 0;
    
    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t value = inpb(port) | (1 << irq_line);
    outpb(port, value);
}

void pic_clear_mask(uint8_t irq_line) {
    uint16_t port = 0;
    
    if(irq_line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq_line -= 8;
    }

    uint8_t value = inpb(port) & ~(1 << irq_line);
    outpb(port, value);
}

void pic_disable(void) {
    outpb(PIC1_DATA, 0xff);
    outpb(PIC2_DATA, 0xff);
}

void pic_remap(uint8_t offset1, uint8_t offset2) {
    // save masks
    uint8_t mask1 = inpb(PIC1_DATA);
    uint8_t mask2 = inpb(PIC2_DATA);

    // Start the initialization sequence
    outpb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outpb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    
    // Master PIC vector offset
    outpb(PIC1_DATA, offset1);

    // Slave PIC vector offset
    outpb(PIC2_DATA, offset2);

    // Tell master PIC that there is a slave PIC at IRQ2 (0000 01000)
    outpb(PIC1_DATA, 4);

    // Tell slave PIC its cascade identity (0000 0010)
    outpb(PIC2_DATA, 2);

    // Have the PICs use 8086 mode (and not 8080 mode)
    outpb(PIC1_DATA, ICW4_8086);
    outpb(PIC2_DATA, ICW4_8086);

    // Restore saved masks
    outpb(PIC1_DATA, mask1);
    outpb(PIC2_DATA, mask2);

    log_info("Remapped Programmable Interrupt Controller!");
}