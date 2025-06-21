#include "../../include/kernel/dma.h"
#include "../../include/kernel/io.h"

void DMA_set_address(uint8_t channel, uint16_t addr){
    uint16_t port = 0;

    switch (channel){
        case 0: port = DMA0_CHAN0_ADDR_REG; break;
        case 1: port = DMA0_CHAN1_ADDR_REG; break;
        case 2: port = DMA0_CHAN2_ADDR_REG; break;
        case 3: port = DMA0_CHAN3_ADDR_REG; break;
        case 4: return;
        case 5: port = DMA1_CHAN5_ADDR_REG; break;
        case 6: port = DMA1_CHAN6_ADDR_REG; break;
        case 7: port = DMA1_CHAN7_ADDR_REG; break;
        default: return;
    }

    port_write_byte(port, addr & 0xFF); /* low byte */
    port_write_byte(port, addr >> 8);   /* high byte */
}

void DMA_set_count(uint8_t channel, uint16_t count){
    uint16_t port;

    switch (channel){
        case 0: port = DMA0_CHAN0_COUNT_REG; break;
        case 1: port = DMA0_CHAN1_COUNT_REG; break;
        case 2: port = DMA0_CHAN2_COUNT_REG; break;
        case 3: port = DMA0_CHAN3_COUNT_REG; break;
        case 4: return;
        case 5: port = DMA1_CHAN5_COUNT_REG; break;
        case 6: port = DMA1_CHAN6_COUNT_REG; break;
        case 7: port = DMA1_CHAN7_COUNT_REG; break;
        default: return;
    }

    port_write_byte(port, count & 0xFF); /* low byte */
    port_write_byte(port, count >> 8);   /* high byte */
}

void DMA_set_page(uint8_t channel, uint8_t page){
    uint16_t port;

    switch(channel){
        case 0: port = DMA_PAGE_CHAN0; break;
        case 1: port = DMA_PAGE_CHAN1; break;
        case 2: port = DMA_PAGE_CHAN2; break;
        case 3: port = DMA_PAGE_CHAN3; break;
        case 4: return;
        case 5: port = DMA_PAGE_CHAN5; break;
        case 6: port = DMA_PAGE_CHAN6; break;
        case 7: port = DMA_PAGE_CHAN7; break;
        default: return;
    }

    port_write_byte(port, page);
}

void DMA_set_mask(uint8_t channel, bool mask){
    if (mask) {
        if (channel <= 4)
            port_write_byte(DMA0_CHANMASK_REG, (1 << (channel - 1)));
        else
            port_write_byte(DMA1_CHANMASK_REG, (1 << (channel - 5)));
        }
    else {
            if (channel <= 4)
                port_write_byte(DMA0_CHANMASK_REG, channel);
            else
                port_write_byte(DMA1_CHANMASK_REG, channel);        
    }
}

void DMA_set_mode(uint8_t channel, uint8_t mode){
    uint8_t dma = !(channel < 4);
    uint8_t indiv_channel = channel - 4 * (dma < 4);

    DMA_set_mask(channel, 1);

    port_write_byte (
        dma ? DMA1_MODE_REG : DMA0_MODE_REG,
        indiv_channel | mode
    );

    DMA_set_mask(channel, 0);
}

void DMA_reset_flipflop(uint8_t dma){
    port_write_byte(
        dma ? DMA1_CLEARBYTE_FLIPFLOP_REG : DMA0_CLEARBYTE_FLIPFLOP_REG,
        0x0 /* doesnt matter what is written */
    );
}

void DMA_reset(){
    port_write_byte(DMA0_TEMP_REG, 0x0);
}

void DMA_unmask_all(){
    port_write_byte(DMA1_UNMASK_ALL_REG, 0x0);
}