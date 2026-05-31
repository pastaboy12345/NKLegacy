#include "keyboard.h"
#include "../../arch/i386/io.h"
#include "../../arch/i386/idt.h"

#define KB_DATA_PORT  0x60
#define KB_BUF_SIZE   256

/* Scancode set 1 -> ASCII lookup (unshifted) */
static const char scancode_ascii[128] = {
    0, 27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,   /* Ctrl */
    'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,   /* Left Shift */
    '\\','z','x','c','v','b','n','m',',','.','/',
    0,   /* Right Shift */
    '*',
    0,   /* Alt */
    ' ',
    0,   /* Caps Lock */
    0,0,0,0,0,0,0,0,0,0,  /* F1-F10 */
    0,   /* Num Lock */
    0,   /* Scroll Lock */
    0,0,0,'-',0,0,0,'+',0,0,0,0,0, /* Keypad */
    0,0, /* unused */
    0,0  /* F11, F12 */
};

/* Shifted scancode -> ASCII */
static const char scancode_ascii_shift[128] = {
    0, 27, '!','@','#','$','%','^','&','*','(',')','_','+','\b',
    '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,
    'A','S','D','F','G','H','J','K','L',':','"','~',
    0,
    '|','Z','X','C','V','B','N','M','<','>','?',
    0,'*',0,' ',0,
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,'-',0,0,0,'+',0,0,0,0,0,
    0,0,0,0
};

/* Circular key buffer */
static volatile char key_buffer[KB_BUF_SIZE];
static volatile int  kb_read  = 0;
static volatile int  kb_write = 0;
static int shift_held = 0;

static void keyboard_callback(registers_t *regs) {
    (void)regs;
    uint8_t scancode = inb(KB_DATA_PORT);

    /* Track shift key state */
    if (scancode == 0x2A || scancode == 0x36) {
        shift_held = 1;
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_held = 0;
        return;
    }

    /* Ignore key releases */
    if (scancode & 0x80) return;

    char c;
    if (shift_held) {
        c = scancode_ascii_shift[scancode];
    } else {
        c = scancode_ascii[scancode];
    }

    if (c) {
        int next = (kb_write + 1) % KB_BUF_SIZE;
        if (next != kb_read) {
            key_buffer[kb_write] = c;
            kb_write = next;
        }
    }
}

void keyboard_init(void) {
    idt_register_handler(33, keyboard_callback);
}

char keyboard_getchar(void) {
    if (kb_read == kb_write) return 0;
    char c = key_buffer[kb_read];
    kb_read = (kb_read + 1) % KB_BUF_SIZE;
    return c;
}
