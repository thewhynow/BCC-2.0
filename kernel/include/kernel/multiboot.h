#ifndef _MULTIBOOT_H
#define _MULTIBOOT_H

#include "../../../libc/include/types.h"


typedef struct {
    uint32_t size;
    uint32_t address_low;
    uint32_t address_high; // ignored on 32-bit
    uint32_t length_low;
    uint32_t length_high; // ignored on 32-bit
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
    uint32_t type;
} __attribute__((__packed__)) mmap_entry_t;

/* like 3/4 of this is not relevant to me but i keep it anyway */
typedef struct {
    uint32_t flags;        // flags
    uint32_t mem_lower;    // lower memory (KB)
    uint32_t mem_upper;    // upper memory (KB)
    uint32_t boot_device;  // boot device ID
    uint32_t cmdline;      // address of kernel command line
    uint32_t mods_count;   // number of modules loaded
    uint32_t mods_addr;    // address of first module structure

    uint32_t irrelevant[4]; // this was really irrelevant XD

    uint32_t mmap_length;  // memory map length
    uint32_t mmap_addr;    // memory map address
    uint32_t drives_length; // drive info length
    uint32_t drives_addr;   // drive info address
    uint32_t config_table;  // ROM configuration table
    uint32_t boot_loader_name; // bootloader name string address
    uint32_t apm_table;     // APM table address
    uint32_t vbe_control_info; // VBE control information
    uint32_t vbe_mode_info; // VBE mode information
    uint32_t vbe_mode;      // VBE mode
    uint32_t vbe_interface_seg;  // VBE interface segment
    uint32_t vbe_interface_off;  // VBE interface offset
    uint32_t vbe_interface_len;  // VBE interface length
} __attribute__((__packed__)) multiboot_info_t;

#ifdef __cplusplus
extern "C" {
#endif
extern multiboot_info_t* multiboot_info;
#ifdef __cplusplus
}
#endif


#endif