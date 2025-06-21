#ifndef _FDC_H
#define _FDC_H

#include "../../../libc/include/types.h"
#include "../../../libc/include/stdlib.h"

void IRQ_FDC_handler();
void FDC_write_cmd(uint8_t command);
void FDC_DMA_init();
void FDC_write_sector();
void FDC_read_init();
void FDC_write_init();
void FDC_write_cmd(uint8_t command);
uint8_t FDC_read_data();
void FDC_check_int(uint8_t *st0, uint8_t *cylinder);
void FDC_start_motor(uint8_t drive);
void FDC_stop_motor();
void FDC_disable();
void FDC_enable();
void FDC_irq_wait();
void FDC_CMD_drive_data(
    uint32_t stepr, uint32_t loadt, uint32_t unloadt, bool dma
);
int FDC_CMD_callibrate(uint8_t drive);
int FDC_CMD_seek(uint8_t cyl, uint8_t head);
void FDC_init();
void *FDC_read_sector(uint32_t lba);

#define FLOPPY_CHANNEL 2
#define FLOPPY_IRQ 6
#define SECTORS_PER_TRACK 18



/**
 * FDC REGISTERS
 * 
 * DIGITAL OUTPUT REGISTER (DOR)
 *  [0:1] = Drive Select (0, 1, 2, 3)
 *  [2] = Reset (0 = reset, 1 = enabled)
 *  [3] = Mode (0 = IRQ, 1 = DMA)
 *  [4] = Drive 0 Motor Control (0 = stop, 1 = start)
 *  [5] = Drive 1 Motor Control 
 *  [6] = Drive 2 Motor Control  
 *  [7] = Drive 3 Motor Control 
 * 
 * MAIN STATUS REGISTER (MSR)
 *  [0] = FDD 0 (0 = not busy, 1 = busy)
 *  [1] = FDD 1
 *  [2] = FDD 2
 *  [3] = FDD 3
 *  [4] = FDC Busy (0 = not busy, 1 = busy)
 *  [5] = FDC !(DMA Mode) (0 = DMA, 1 = IRQ)
 *  [6] = DIO (0 = FDC expecting data, 1 = FDC has data)
 *  [7] = MQR (0 = Data Register not ready, 1 = Data Register ready)
 * 
 * CONFIGURATION CONTROL REGISTER (CCR)
 *  [0:2] = Data Rate Select
 *      00 -> 500 KBPS
 *      10 -> 250 KBPS
 *      01 -> 300 KBPS
 *      11 -> 1   MBPS
 */

typedef enum {
    FDC_DOR	 = 0x3F2,
	FDC_MSR	 = 0x3F4,
	FDC_FIFO = 0x3F5,
	FDC_CTRL = 0x3F7
} FDC_IO;

typedef enum {
    FDC_DOR_MASK_DRIVE0		  = 0b00000000,
	FDC_DOR_MASK_DRIVE1		  =	0b00000001,
	FDC_DOR_MASK_DRIVE2		  =	0b00000010,
	FDC_DOR_MASK_DRIVE3		  =	0b00000011,
	FDC_DOR_MASK_RESET		  =	0b00000100,
	FDC_DOR_MASK_DMA		  =	0b00001000,
	FDC_DOR_MASK_IRQ		  =	0b00000000,
	FDC_DOR_MASK_DRIVE0_MOTOR =	0b00010000,
	FDC_DOR_MASK_DRIVE1_MOTOR =	0b00100000,
	FDC_DOR_MASK_DRIVE2_MOTOR =	0b01000000,
	FDC_DOR_MASK_DRIVE3_MOTOR =	0b10000000
} FDC_DOR_MASK;

typedef enum {
    FDC_MSR_MASK_DRIVE0_SEEK    = 0b00000001,
	FDC_MSR_MASK_DRIVE1_SEEK    = 0b00000010,
	FDC_MSR_MASK_DRIVE2_SEEK    = 0b00000100,
	FDC_MSR_MASK_DRIVE3_SEEK    = 0b00001000,
	FDC_MSR_MASK_BUSY   	    = 0b00010000,
	FDC_MSR_MASK_IRQ_MODE	    = 0b00100000,
    FDC_MSR_MASK_HAS_DATA       = 0b01000000,
    FDC_MSR_MASK_DATA_READY = 0b10000000
} FDC_MSR_MASK;

/**
 * EXTENDED COMMAND BITS
 * MultiTrack Operation (M)
 *  0 = one track, 1 = both tracks
 * FM/MFM Mode Setting (F)
 *  0 = FM (Single Density), 1 = MFM (Double Density)
 * Skip Mode Setting (S)
 *  0 = Skip Deleted Address Marks, 1 = Do Not Skip Deleted Address marks
 * Head Number (HD)
 * Drive Number Bits [DR0:DR1] 
 *  00 -> Drive 0
 *  01 -> Drive 1
 *  10 -> Drive 2
 *  11 -> Drive 3
 */

typedef enum {
    /**
     * Read / Write Sector
     *  reads or writes a sector from/to the FDD into the DMA-specified memory
     *  format: M F 0 0 1 1 0
     *  parameters:
     *      x x x x x HD DR1 DR0
     *      Cylinder
     *      Head
     *      Sector Number
     *      Sector Size
     *      Track Length / Max Sector Number
     *      Length of GAP3
     *      Data Length
     *  return:
     *      ST0
     *      ST1
     *      ST2
     *      Current Cylinder
     *      Current Head
     *      Sector Number
     *      Sector Size
     */
    FDC_CMD_READ_SECT	 = 0x6,
	FDC_CMD_WRITE_SECT	 = 0x5,

    /**
     * Fix Drive Data
     *  pass controlling info about the mechanical drive to FDC
     *  format: 0 0 0 0 0 0 1 1
     *  parameters:
     *      S S S S H H H H   : S = Step Rate, H = Head Unload Time
     *      H H H H H H H NDM : H = Head Load Time, NDM = !(DMA Mode)
     *              
     *      Step Rate, Head Unload Time, Head Load Time dependent on Data Transfer Rate in CCR
     *      https://www.isdaman.com/alsos/hardware/fdc/floppy.htm 
     *  return: None
     */
	FDC_CMD_SPECIFY		 = 0x3,

    /**
     * Check Drive Status
     *  format: 0 0 0 0 0 1 0 0
     *  parameters:
     *      x x x x x HD DR1 DR0
     *  return:
     *      Status Register 3
     */
	FDC_CMD_CHECK_STAT	 = 0x4,

    /**
     * Calibrate Drive
     *  moves the read/write head to cylinder 0, check if on right track after completion
     *  format:  0 0 0 0 0 1 1 1
     *  parameters:
     *      x x x x x 0 DR1 DR0
     *  return: None
     */
	FDC_CMD_CALIBRATE	 = 0x7,
	
    /**
     * Check Interrupt Status
     *  check the status of the FDC when interrupt returns
     *  format: 0 0 0 0 1 0 0 0
     *  parameters: None
     *  return:
     *      Status Register 0
     *      Current Cylinder
     */
    FDC_CMD_CHECK_INT	 = 0x8,

    /**
     * Seek Head
     *  move the read/write head to a specific cylinder, check if on right one after completion
     *  format: 0 0 0 0 1 1 1 1
     *  parameters:
     *      x x x x x HD DR1 DR0
     *      Cylinder
     *  return: None
     */
	FDC_CMD_SEEK		 = 0xF,
} FDC_COMMAND;

typedef enum {
    FDC_CMD_EXT_SKIP       = 0b00100000,
	FDC_CMD_EXT_DENSITY    = 0b01000000,
	FDC_CMD_EXT_MULTITRACK = 0b10000000
} FDC_COMMAND_EXT;

typedef enum {
    FDC_CMD_GAP3_LENGTH_STD  = 42,
	FDC_CMD_GAP3_LENGTH_5_14 = 32,
	FDC_CMD_GAP3_LENGTH_3_5  = 27
} FDC_COMMAND_GAP3_LEN;

typedef enum {
    FDC_CMD_DTL_128  = 0,
	FDC_CMD_DTL_256  = 1,
	FDC_CMD_DTL_512  = 2,
	FDC_CMD_DTL_1024 = 4
} FDC_COMMAND_DTL;
#endif