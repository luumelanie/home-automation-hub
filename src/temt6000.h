#ifndef TEMT6000_H
#define TEMT6000_H

void adc_init(void); 
unsigned int adc_read(void);
void display_light_level(unsigned int light_value);
int lights_control(unsigned int light_value, unsigned int hrs, unsigned int mins, unsigned int secs);

#endif

