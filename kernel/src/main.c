#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

// Set the base revision to 3 (or higher) to match modern Limine standards
__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(3);

// Request a graphical framebuffer from the bootloader
// Request a graphical framebuffer from the bootloader
__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,  // <-- Added _ID here
    .revision = 0
};


// Halt and catch fire function
static void hcf(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

// Kernel Entry Point
void _start(void) {
    // Ensure the bootloader successfully provided a framebuffer
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Fetch the first available framebuffer
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Clear the screen and draw a 200x200 pixel white square in the top left corner
    for (uint64_t y = 0; y < 200; y++) {
        for (uint64_t x = 0; x < 200; x++) {
            // Calculate pixel offset (32-bit color: 4 bytes per pixel)
            uint32_t *fb_ptr = (uint32_t *)framebuffer->address;
            fb_ptr[y * (framebuffer->pitch / 4) + x] = 0xFFFFFFFF; // White pixel
        }
    }

    // Hang safely
    hcf();
}
