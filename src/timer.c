#include "lpc24xx.h"
#include "lcd/lcd_grph.h"
#include <stdio.h>

// volatile unsigned int seconds = 0;

// moved into timer_asm.s
// void init_timer_polling(void) {
//     PCONP |= (1 << 1);        // Power up Timer0
//     PCLKSEL0 &= ~(3 << 2);    // Clear PCLK for Timer0
//     PCLKSEL0 |=  (1 << 2);    // Set to CCLK (13 MHz)

//     T0TCR = 0x02;             // Reset timer
//     T0PR  = 13000 - 1;        // T0PR  = 72000 - 1;       
//     T0TC  = 0;                // Clear timer counter
//     T0TCR = 0x01;             // Enable timer
// }

// int timer_update_seconds(void) {
//     static unsigned int last_tick = 0;
//     unsigned int tick = T0TC;

//     if (tick - last_tick >= 1000) {  
//         seconds++;
//         last_tick = tick;
//         return 1;  // Signal: update display
//     }
//     return 0;      // Signal: do nothing
// }

// unsigned int get_seconds(void) {
//     return seconds;
// }

void display_time(unsigned int hrs, unsigned int mins, unsigned int secs) {
    char buffer[32];
    sprintf(buffer, "Time: %02u:%02u:%02u", hrs, mins, secs);
    lcd_putString(10, 30, (unsigned char *)buffer);
}
