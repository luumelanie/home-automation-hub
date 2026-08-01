#include "LPC24xx.h"
#include "temt6000.h"
#include "lcd/lcd_grph.h"
#include <stdio.h>

#define TEMT6000_ADC_CH 1         // Channel 1 (AD0.1)

#define LIGHT_THRESHOLD_LOW    300   // Below this = blinds close/lights on
#define LIGHT_THRESHOLD_HIGH   600   // Above this = blinds open/lights off

void adc_init(void) {
    // Power up ADC
    PCONP |= (1 << 12);

    // Configure P0.24 as AD0.1
    PINSEL1 &= ~(3 << 16);        // Clear bits 17:16
    PINSEL1 |= (1 << 16);         // Set bits to 01

    // ADC Configuration
    AD0CR = (1 << TEMT6000_ADC_CH) |  // Select channel
            (4 << 8)              |   // Clock divider (PCLK/5) ~13Mhz
            (1 << 21);                // Enable ADC
}

unsigned int adc_read(void){
	  AD0CR &= ~(1 << 24); 
    AD0CR |= (1 << 24); // Start conversion now

    // Wait for conversion complete
    while (!(AD0GDR & (1UL << 31))){
		}

    // Extract 10-bit result
    return (AD0GDR >> 6) & 0x3FF;
}
void display_light_level(unsigned int light_value){
		char buffer[32];
		sprintf(buffer, "Light:%02u", light_value);
		lcd_putString(160, 60, (unsigned char *)buffer);

}

int lights_control(unsigned int light_value, unsigned int hrs, unsigned int mins, unsigned int secs){
		if (light_value <= LIGHT_THRESHOLD_LOW) {//lights turn on between 5pm - 11pm when low light levels are first detected
					lcd_putString(10, 200, (unsigned char *)"Light ON(sensor)");
					return 1;
		} 
	return 0;
}


