#ifndef _DMA_H
#define _DMA_H

#include "../../../libc/include/types.h"

/**
 * DMA_COMMAND_REGISTER
 *  [2] = Controller Enable (0 = OFF)
 * 
 * DMA_MODE_REGISTER (disable channel before writing)
 *  [0:1] = Channel Select (0, 1, 2, 3)
 *  [2:3] = Transfer Type
 *      00 -> self test
 *      01 -> write transfer
 *      10 -> read transfer
 *      11 -> invalid
 *  [4] = Auto Reinitialize (not supported by virtual machines)
 *  [5] = IDEC
 *  [6:7] = Mode
 *      00 -> transfer on demand
 *      01 -> single DMA transfer
 *      10 -> block DMA transfer
 *      11 -> cascade mode 
 *    
 * DMA_CHANNEL_MASK_REGISTER
 *  [0:1] = Channel Select (0, 1, 2, 3)
 *  
 * DMA_MASK_REGIStER
 *  [0] = Channel 0
 *  [1] = Channel 1
 *  [2] = Channel 2
 *  [3] = Channel 3
 */

typedef enum {
	DMA0_COMMAND_REG            = 0x8, /* write */
	DMA0_CHANMASK_REG           = 0xA, /* write */ 
	DMA0_MODE_REG               = 0xB, /* write */
	DMA0_CLEARBYTE_FLIPFLOP_REG = 0xC, /* write */
	DMA0_TEMP_REG               = 0xD, /* read */
	DMA0_MASTER_CLEAR_REG       = 0xD, /* write */
	DMA0_CLEAR_MASK_REG         = 0xE, /* write */
	DMA0_MASK_REG               = 0xF  /* write */
} DMA_0_IO;

typedef enum {
	DMA1_STATUS_REG             = 0xD0, /* read */
	DMA1_COMMAND_REG            = 0xD0, /* write */
	DMA1_REQUEST_REG            = 0xD2, /* write */
	DMA1_CHANMASK_REG           = 0xD4, /* write */
	DMA1_MODE_REG               = 0xD6, /* write */
	DMA1_CLEARBYTE_FLIPFLOP_REG = 0xD8, /* read */
	DMA1_INTER_REG              = 0xDA, /* write*/
	DMA1_UNMASK_ALL_REG         = 0xDC, /* write */
	DMA1_MASK_REG               = 0xDE  /* write */
} DMA_1_IO;

typedef enum {
    DMA_MODE_MASK_SELECT        = 0b00000011,

	DMA_MODE_MASK_TRANSFER      = 0b00001100,
	DMA_MODE_SELF_TEST          = 0b00000000,
	DMA_MODE_READ_TRANSFER      = 0b00000100,
	DMA_MODE_WRITE_TRANSFER     = 0b00001000,

	DMA_MODE_MASK_AUTO          = 0b00010000,
	DMA_MODE_MASK_IDEC          = 0b00100000,

	DMA_MODE_MASK               = 0b11000000,
	DMA_MODE_TRANSFER_ON_DEMAND = 0b00000000,
	DMA_MODE_TRANSFER_SINGLE    = 0b01000000,
	DMA_MODE_TRANSFER_BLOCK     = 0b10000000,
	DMA_MODE_TRANSFER_CASCADE   = 0b11000000
} DMA_MODE_REG_MASK;

/**
 * DMA_*_CHANNEL_IO
 * 
 * initializes each of the different channels
 * CHAN*_ADDR_REG:  the PHYSICAL address to start transferring data
 * CHAN*_COUNT_REG: the amount of data to transfer
 */

typedef enum {
    DMA0_CHAN0_ADDR_REG  = 0,
	DMA0_CHAN0_COUNT_REG = 1,
	DMA0_CHAN1_ADDR_REG  = 2,
	DMA0_CHAN1_COUNT_REG = 3,
	DMA0_CHAN2_ADDR_REG  = 4,
	DMA0_CHAN2_COUNT_REG = 5,
	DMA0_CHAN3_ADDR_REG  = 6,
	DMA0_CHAN3_COUNT_REG = 7,
} DMA_0_CHANNEL_IO;

typedef enum {
    DMA1_CHAN4_ADDR_REG  = 0xC0,
	DMA1_CHAN4_COUNT_REG = 0xC2,
	DMA1_CHAN5_ADDR_REG  = 0xC4,
	DMA1_CHAN5_COUNT_REG = 0xC6,
	DMA1_CHAN6_ADDR_REG  = 0xC8,
	DMA1_CHAN6_COUNT_REG = 0xCA,
	DMA1_CHAN7_ADDR_REG  = 0xCC,
	DMA1_CHAN7_COUNT_REG = 0xCE,
} DMA_1_CHANNEL_IO;

/**
 * DMA_PAGE_*
 * 
 * ports used for the page register for each channel
 * 
 * extends the original address register by 8 bits 
 * for 24-bit addressing
 */

typedef enum {
	DMA_PAGE_CHAN2 = 0x81,
	DMA_PAGE_CHAN3 = 0x82,
	DMA_PAGE_CHAN1 = 0x83,
    DMA_PAGE_CHAN0 = 0x87,
	DMA_PAGE_CHAN6 = 0x89,
	DMA_PAGE_CHAN7 = 0x8A,
	DMA_PAGE_CHAN5 = 0x8B,
} DMA_PAGE_REG;

void DMA_set_address(uint8_t channel, uint16_t addr);
void DMA_set_count  (uint8_t channel, uint16_t count);
void DMA_set_page   (uint8_t channel, uint8_t page);
void DMA_set_mask   (uint8_t channel, bool mask);
void DMA_set_mode   (uint8_t channel, uint8_t mode);

void DMA_reset_flipflop(uint8_t dma);
void DMA_reset();
void DMA_unmask_all();

#define DMA_SET_READ(channel) \
    DMA_set_mode(channel, DMA_MODE_READ_TRANSFER | DMA_MODE_TRANSFER_SINGLE | DMA_MODE_MASK_AUTO);

#define DMA_SET_WRITE(channel) \
    DMA_set_mode(channel, DMA_MODE_WRITE_TRANSFER | DMA_MODE_TRANSFER_SINGLE | DMA_MODE_MASK_AUTO);


#endif