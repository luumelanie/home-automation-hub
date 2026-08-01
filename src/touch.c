#include "touch.h"
#include "lpc24xx.h"
#include "delay.h"

#define CS_PIN  0x00100000

// Control bytes 
#define X_POSITION     0xD8  // 11011000
#define Y_POSITION     0x98  // 10011000

// Control bytes for pressure
#define Z1_POSITION    0xB0  // 10110000
#define Z2_POSITION    0xC0  // 11000000

// Pressure scale
#define K 1

static unsigned char touch_read(unsigned char command);
static int pressure;

void touch_init(void)
{

		FIO0DIR |= CS_PIN;
	
		// SCK (P0.15)
		PINSEL0 &= ~(3 << 30);  
		PINSEL0 |=  (3 << 30);  

		// MISO (P0.17)
		PINSEL1 &= ~(3 << 2);   
		PINSEL1 |=  (3 << 2);  

		// MOSI (P0.18)
		PINSEL1 &= ~(3 << 4);
		PINSEL1 |=  (3 << 4);

		// Configure SPI controller
		S0SPCCR = 0x24;      // SPI clock = PCLK / 36 = 2 MHz
		S0SPCR = 0x093C;     // 9-bit transfer, master, CPOL=1, CPHA=1, MSB first
												 
}

void touch_read_xy(char *x, char *y, char *z1, char *z2) {
    *x = touch_read(0xD8);  //  11010011
    *y = touch_read(0x98);  // 10010011
		*z1 = touch_read(0xB8); // pressure z1
		*z2 = touch_read(0xC8); //pressure z2
}

static unsigned char touch_read(unsigned char command)
{
    unsigned short result;
	
    FIO0CLR = CS_PIN;

    S0SPDR = command;
    while (!(S0SPSR & (1 << 7))); // Wait for SPIF
    
    S0SPDR = 0x00; 
    while (!(S0SPSR & (1 << 7)));
    result = S0SPDR & 0x1FF;  
		
    FIO0SET = CS_PIN;
	
    return result;
}	

