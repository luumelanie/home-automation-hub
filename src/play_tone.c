#include "lpc24xx.h"

// Initialize DAC.
// This should be the same as the previous exercise.
void setup_DAC(void) {
    // Set P0.26 to DAC function (AOUT) — PINSEL1[21:20] = 0b10
    PINSEL1 &= ~(0x3 << 20);  // Clear bits 21:20
    PINSEL1 |=  (0x2 << 20);  // Set bits 21:20 to 0b10 (DAC function)
}

// udelay should delay the processor for 'delay_in_us' number of microseconds.
// * LPC24XX.h contains a definition for the PCLK, "Fpclk"
//   use this definition for PCLK as defined in the manual.
// * Use Timer 0. This means you MUST use the prefix T0 for every control
//   register, i.e. T0TCR.
// * Make sure you reset the timer, and when you start the timer ENSURE you
//   set the reset bit back to zero!
void udelay(unsigned int delay_in_us) {
    // 1. Reset Timer 0
    T1TCR = 0x02;  // Set bit 1 to reset

    // 2. Set prescaler to get 1 µs resolution
    T1PR = 71;  // Typically: 71 for 72 MHz

    // 3. Clear timer counter
    T1TC = 0;

    // 4. Clear reset bit and start timer
    T1TCR = 0x01;

    // 5. Wait until TC reaches the desired delay
    while (T1TC < delay_in_us);

    // 6. Stop timer
    T1TCR = 0x00;
}
	

// play_tone should play a tone for 'duration' number of microseconds at the pitch
// given by 1/'period'. Played at the volume given by 'vol', which ranges from																   
// 0 (off) to 0x3FF (max volume).																		   

void play_tone(unsigned int duration, int period, int vol) {
    unsigned int time_elapsed = 0;

    // Each full period consists of two halves: high and low
    while (time_elapsed < duration) {

        DACR = vol << 6;                // Set DAC output high, shifts value to bits 15:6
        udelay(period / 2);             // Delay for half the period

        DACR = 0 << 6;                  // Set DAC output low
        udelay(period / 2);             // Delay for the other half

        time_elapsed += period;        // Track how much time has passed
    }

}
