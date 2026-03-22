#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define PIT_FREQUENCY 1000  // 1000 Hz = 1ms per tick

void timer_init(void);
uint64_t timer_get_ticks(void);
void timer_sleep(uint64_t ms);

#endif // TIMER_H
