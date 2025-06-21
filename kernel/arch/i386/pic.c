#include "../../include/kernel/pic.h"

/*
Initialization Control Word 1
    0   IC4     if set, PIC expects to recieve ICW4 during init
    1   SGNL    if set, only 1 PIC in system; if unset PIC is cascaded w/ slave PICs
                and ICW3 must be sent to controller
    2   ADI     call address interval, set: 4, not set: 8; ignored on x86, set 0
    3   LTIM    if set, operate in level-triggered mode; if unset, operate in edge-triggered mode
    4   INIT    set to 1 to initialize PIC
    5-7         ignored on x86, set 0
*/              
typedef enum {
    PIC_ICW1_ICW4       = 0b00000001,
    PIC_ICW1_SINGLE     = 0b00000010,
    PIC_ICW1_INTERVAL4  = 0b00000100,
    PIC_ICW1_LEVEL      = 0b00001000,
    PIC_ICW1_INITIALIZE = 0b00010000
} PIC_ICW1;

/*              
Initialization Control Word 4
    0   uPM     if set, PIC is in 80x86 mode, if not set, in MCS-80/86 mode
    1   AEOI    if set, on last interrupt acknowledge pulse, controller automatically
                performs end of interrupt operation
    2   M/S     only use if BUF is set; if set, selects buffer master; otherwise selects buffer slave
    3   BUF     if set, controller operates in buffered mode
    4   SFNM    specially fully nested mode; used in systems with large number of cascaded controllers
    5-7         reserved, set 0
*/
typedef enum {
    PIC_ICW4_8086          = 0x1,
    PIC_ICW4_AUTO_EOI      = 0x2,
    PIC_ICW4_BUFFER_MASTER = 0x4,
    PIC_ICW4_BUFFER_SLAVE  = 0x0,
    PIC_ICW4_BUFFERED      = 0x8,
    PIC_ICW4_SFNM          = 0x10
} PIC_ICW4;

#define PIC_CMD_END_OF_INT 0x20

void PIC_configure(uint8_t offset_pic1, uint8_t offset_pic2){
    // control word 1
    port_write_byte(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    io_wait();
    port_write_byte(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    io_wait();

    // control word 2
    port_write_byte(PIC1_DATA_PORT, offset_pic1);
    io_wait();
    port_write_byte(PIC2_DATA_PORT, offset_pic2);
    io_wait();

    // control word 3
    port_write_byte(PIC1_DATA_PORT, 0x4); // tell PIC1 that it has a slave at IRQ2 (0000 0100)
    io_wait();
    port_write_byte(PIC2_DATA_PORT, 0x2); // tell PIC2 its cascade identity (0000 0010)
    io_wait();

    // control word 4
    port_write_byte(PIC1_DATA_PORT, PIC_ICW4_8086);
    io_wait();
    port_write_byte(PIC2_DATA_PORT, PIC_ICW4_8086);
    io_wait();

    // mask all interrupts
    port_write_byte(PIC1_DATA_PORT, 0xFF);
    io_wait();
    port_write_byte(PIC2_DATA_PORT, 0xFF);
    io_wait();
}

void PIC_mask(int irq){
    uint8_t port;

    if (irq < 8)
        port = PIC1_DATA_PORT;
    else {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = port_read_byte(port);
    port_write_byte(port, mask | (1 << irq));
}

void PIC_unmask(int irq){
    uint8_t port;

    if (irq < 8)
        port = PIC1_DATA_PORT;
    else {
        irq -= 8;
        port = PIC2_DATA_PORT;
    }

    uint8_t mask = port_read_byte(port);
    port_write_byte(port, mask & ~(1 << irq));
}

#define PIC_CMD_READ_IRR 0x0A
#define PIC_CMD_READ_ISR 0x0B

uint16_t PIC_readIRQ_request_reg(){
    port_write_byte(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    port_write_byte(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);

    return port_read_byte(PIC2_COMMAND_PORT) | (port_read_byte(PIC1_COMMAND_PORT) << 8); 
}

uint16_t PIC_readIRQ_inservice_reg(){
    port_write_byte(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    port_write_byte(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);

    return port_read_byte(PIC2_COMMAND_PORT) | (port_read_byte(PIC1_COMMAND_PORT) << 8); 
}

void PIC_end_of_int(int irq){
    if (irq >= 8)
        port_write_byte(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INT);
    port_write_byte(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INT);
}