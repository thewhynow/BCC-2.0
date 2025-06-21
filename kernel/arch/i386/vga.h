#ifndef ARCH_I386_VGA_H
#define ARCH_I386_VGA_H

#include <stdint.h>

typedef enum {
    VGA_BLACK,
    VGA_BLUE,
    VGA_GREEN,
    VGA_DARK_CYAN,
    VGA_RED,
    VGA_MAGENTA,
    VGA_ORANGE,
    VGA_LIGHT_GRAY,
    VGA_DARK_GRAY,
    VGA_PURPLE,
    VGA_LIGHT_GREEN,
    VGA_LIGHT_CYAN,
    VGA_LIGHT_RED,
    VGA_PINK,
    VGA_YELLOW,
    VGA_WHITE
} vga_color_t;

static inline uint8_t vga_color(vga_color_t fg, vga_color_t bg){
    return fg | bg << 4;
}

static inline uint16_t vga_entry(char c, uint8_t color){
    return (uint16_t)c | (uint16_t)color << 8;
}

#endif