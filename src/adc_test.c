// adc_test.c  (no main here)
#include "lpc24xx.h"
#include <stdint.h>
#include "adc_test.h"

void setupADC(void) {
    // Power up ADC
    PCONP |= (1u << 12);

    // P0.25 -> AD0.2  (PINSEL1 bits 19:18 = 01)
    PINSEL1 &= ~(3u << 18);
    PINSEL1 |=  (1u << 18);
}

int readADC(void) {
    const unsigned CLKDIV = 3; // 18 MHz / (3+1) = 4.5 MHz
    uint32_t base_cr;
    int sample;

    base_cr = (1u << 2)       // SEL: channel 2 (AD0.2)
            | (CLKDIV << 8)   // CLKDIV
            | (1u << 21);     // PDN

    AD0CR = base_cr | (1u << 24); // START=001

    while ((AD0DR2 & (1u << 31)) == 0) {
        // wait DONE
    }

    sample = (AD0DR2 >> 6) & 0x03FF; // 10-bit result
    AD0CR = base_cr;                 // stop/start bits cleared
    return sample;                   // 0..1023
}
