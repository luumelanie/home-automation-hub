#include "lpc24xx.h"
#include <stdint.h>
#include "light_sensor.h"
#include <stdio.h>
#include "lcd/lcd_grph.h"   

void setupADC_Light(void) {
    PCONP |= (1 << 12);                 

    // Ensure ADC PCLK = CCLK/4, PCLKSEL0 bits 25:24 = 00.
    PCLKSEL0 &= ~(3 << 24);

    // P0.24 -> AD0.1  (PINSEL1 bits 17:16 = 01)
    PINSEL1 &= ~(3 << 16);
    PINSEL1 |=  (1 << 16);
}

int readADC_Light(void) {
    const unsigned CLKDIV = 3;
    uint32_t base_cr;
    int sample;

    // Build control word:
    // - SEL: channel 1 (AD0.1) -> bit 1
    // - CLKDIV: as above
    // - PDN: enable ADC 
    base_cr = (1 << 1)        // select channel 1
            | (CLKDIV << 8)   // ADC clock divisor
            | (1 << 21);      // PDN = 1

    // Start conversion
    AD0CR = base_cr | (1 << 24);

    // Wait for completion 
    while ((AD0DR1 & (1 << 31)) == 0) { }

    // 10-bit result is bits [15:6]
    sample = (AD0DR1 >> 6) & 0x03FF;

    // clear start bits
    AD0CR = base_cr;

    return sample; 
}

void light_sensor_display(void) {
    unsigned int codeL = (unsigned int)readADC_Light();
    unsigned int mvL   = (codeL * 3300u) / 1023u;
    static unsigned char lineL1[44];
    static unsigned char lineL2[44];

    lcd_fontColor(WHITE, BLACK);
    sprintf((char*)lineL1, "Light: %4u",    codeL);
    lcd_putString(150, 80, lineL1);

    sprintf((char*)lineL2, "Light: %4u mV", mvL);
    lcd_putString(150, 90, lineL2);
}
