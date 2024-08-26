#ifndef INIT_H
#define INIT_H

#include <stddef.h>
#include <stdint.h>
#include "limine.h"

// Define global variables
extern uint32_t *framebuffer_ptr;
extern size_t framebuffer_width;
extern size_t framebuffer_height;
extern size_t framebuffer_pitch;

// Limine request structures
extern volatile struct limine_framebuffer_request framebuffer_request;

// Request markers
extern volatile struct limine_requests_start_marker requests_start_marker;
extern volatile struct limine_requests_end_marker requests_end_marker;

extern volatile struct limine_memmap_request memmap_request;

#endif // INIT_H
