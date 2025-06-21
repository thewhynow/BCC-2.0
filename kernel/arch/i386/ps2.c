#include "../../include/kernel/ps2.h"

typedef enum {
    PS2_ESCAPE = 0x01,
    PS2_NUM_1,
    PS2_NUM_2,
    PS2_NUM_3,
    PS2_NUM_4,
    PS2_NUM_5,
    PS2_NUM_6,
    PS2_NUM_7,
    PS2_NUM_8,
    PS2_NUM_9,
    PS2_NUM_0,
    PS2_MINUS,
    PS2_EQUAL,
    PS2_BACKSPACE,
    PS2_TAB,
    PS2_CAPITAL_Q,
    PS2_CAPITAL_W,
    PS2_CAPITAL_E,
    PS2_CAPITAL_R,
    PS2_CAPITAL_T,
    PS2_CAPITAL_Y,
    PS2_CAPITAL_U,
    PS2_CAPITAL_I,
    PS2_CAPITAL_O,
    PS2_CAPITAL_P,
    PS2_OPEN_BRACKET,
    PS2_CLOSE_BRACKET,
    PS2_ENTER,
    PS2_LEFT_CTRL,
    PS2_CAPITAL_A,
    PS2_CAPITAL_S,
    PS2_CAPITAL_D,
    PS2_CAPITAL_F,
    PS2_CAPITAL_G,
    PS2_CAPITAL_H,
    PS2_CAPITAL_J,
    PS2_CAPITAL_K,
    PS2_CAPITAL_L,
    PS2_SEMICOLON,
    PS2_SINGLE_QUOTE,
    PS2_BACKTICK,
    PS2_LEFT_SHIFT,
    PS2_BACKSLASH,
    PS2_CAPITAL_Z,
    PS2_CAPITAL_X,
    PS2_CAPITAL_C,
    PS2_CAPITAL_V,
    PS2_CAPITAL_B,
    PS2_CAPITAL_N,
    PS2_CAPITAL_M,
    PS2_COMMA,
    PS2_PERIOD,
    PS2_FORW_SLASH,
    PS2_RIGHT_SHIFT,
    PS2_KEYPAD_STAR,
    PS2_LEFT_ALT,
    PS2_SPACE,
    PS2_CAPS_LOCK,
} PS2_SCANCODES;

char KEYBOARD_MAP[0x58] = {
    '\0',
    '\0',
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
    '-', '=',
    '\0', '\0',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
    '[', ']',
    '\0', '\0',
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
    ';', '\'', '`',
    '\0', 
    '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm',
    ',', '.', '/',
    '\0',
    '*',
    '\0',
    ' ',
    '\0'
};

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_IRQ 0x1

static char ps2_stdin[PS2_STDIN_SIZE];

static char keycode;

void IRQ_keyboard_handler(){
    PIC_end_of_int(KEYBOARD_IRQ);
    uint8_t status = port_read_byte(KEYBOARD_STATUS_PORT);
    if (status & 0x1)
        keycode = port_read_byte(KEYBOARD_DATA_PORT);
}

char* PS2_read(){
    PIC_unmask(KEYBOARD_IRQ);

    while (keycode != PS2_ENTER){
        asm volatile ("hlt");
        if (0 < keycode && keycode <= 0x58){
            if (keycode == PS2_BACKSPACE){
                if (ps2_stdin[0]){
                    terminal_delchar();
                    ps2_stdin[strlen(ps2_stdin) - 1] = '\0';
                }
            }
            else {
                char c = KEYBOARD_MAP[(size_t)keycode];
                if (c && strlen(ps2_stdin) < PS2_STDIN_SIZE){
                    strncat(ps2_stdin, &c, 1);
                    terminal_putchar(c);
                }
            }
        }
    }

    keycode = 0;

    PIC_mask(KEYBOARD_IRQ);
    
    return ps2_stdin;    
}

void PS2_flush(){
    memset(ps2_stdin, 0, PS2_STDIN_SIZE);
}