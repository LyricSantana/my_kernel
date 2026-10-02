 #include <stdint.h>
 #include <stddef.h>
 #include <stdbool.h>
 #include "limine.h"

//lines 12-24 is code from limine-c-template repo

// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

//learned that "halt and catch fire" is an actual term lol
//halts the cpu safely
static void hcf(void) {
    //injects raw assembly instructions. 
    //cli stands for "clear interrupt flag"
    //disables ardware interrupts and the cpu will ignore signals from stuff like keyboards or mice
    __asm__ volatile ("cli");
    for (;;) {
        //hlt(halt)
        //puts the cpu core into a low-power, sleeping state
        //by disabling interrupts and calling hlt inside an infinite loop, the cpu freezes  until the computer is forced to reboot
        __asm__ volatile ("hlt")
    }
}

//limine has no built in terminal console so you have to draw letters yourself
/*
hex -> binary
1 is colored, 0 is not
0x66 -> 0 1 1 0 0 1 1 0
0x66 -> 0 1 1 0 0 1 1 0
0x66 -> 0 1 1 0 0 1 1 0
0x7E -> 0 1 1 1 1 1 1 0 horizontal bridge
0x66 -> 0 1 1 0 0 1 1 0
0x66 -> 0 1 1 0 0 1 1 0
0x66 -> 0 1 1 0 0 1 1 0
0x66 -> 0 1 1 0 0 1 1 0
*/
static const uint8_t font8x8[128][8] = {
    ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x66},
    ['E'] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
    ['l'] = {0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['W'] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    ['r'] = {0x00, 0x00, 0x2C, 0x32, 0x30, 0x30, 0x30, 0x00},
    ['d'] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
};

//horrible function yucky ew
void draw_char(struct limine_framebuffer *fb, int x, int y, char c, uint32_t color) {
    //points directly to the graphics card's video ram
    volatile uint32_t *fb_ptr = (volatile uint32_t *)fb->address;
    //pitch is the size of one horizontal row of pixels in bytes. each pixel takes up 4 bytes. dividing the pitch by 4 gives how many pixels fit across the screen horizontally
    uint64_t pitch_pixels = fb->pitch / 4;

    //calculates the exact index in VRAM memory for any coordinate on the screen: Index = (Y Coordinate × Width of Screen) + X Coordinate
    for (int row = 0; row < 8; row++) {
        uint8_t font_row = font8x8[(uint8_t)c][row];
        for (int col = 0; col < 8; col++) {
            if ((font_row >> (7 - col)) & 1) {
                fb_ptr[(y + row) * pitch_pixels + (x + col)] = color;
            }
        }
    }
}

void draw_string(struct limine_framebuffer *fb, int x, int y, const char *str, uint32_t color) {
    for (int i = 0; str[i] != '\0'; i++) {
        draw_char(fb, x + (i*8), y, str[i], color);
    }
}