#include "bubbles.h"
#include "touch.h"
#include "lcd/lcd_grph.h"
#include <stdio.h>
#include <stdint.h>
#include "lpc24xx.h"
#include "timer.h"
#include "blinds.h"
#include "doorbell_new.h"
#include "smartplug.h"
#include "play_tone.h"
#include "temt6000.h"
#include "delay.h"
#include "manual_timer.h"
#include "light_sensor.h"
#include "ui.h"

// declarations to silence warnings 
void display_time(unsigned int hrs, unsigned int mins, unsigned int secs);
void blind_1_timer_task(unsigned int hrs, unsigned int mins, unsigned int secs);
void blind_2_light_task(void);

#define LEFT_BUTTON     (1 << 10)
#define RIGHT_BUTTON    (1 << 11)
#define LEFT_RED        (1 << 16)
#define LEFT_GREEN      (1 << 17)
#define LEFT_BLUE       (1 << 18)
#define RIGHT_RED       (1 << 19)
#define RIGHT_GREEN     (1 << 20)
#define RIGHT_BLUE      (1 << 21)

// Touch press threshold and release debounce 
#define RELEASE_FRAMES      3   // require N consecutive not-pressed frames 


int main(void) {
		// Timer variables
    unsigned int hrs, mins, secs, tick;
		
		// Doorbell variables
    int last_btn_state = (FIO0PIN & RIGHT_BUTTON) ? 1 : 0;
    int curr_btn_state;
		
		// Light and Blind variables
    int light_on = 0;
    int blind_1_state = 0;
    int blind_2_state = 0;

    // Touchscreen variables
    int touch_ready = 1;         					 // 1 = can accept a new press and 0 = no new press
    int release_frames = RELEASE_FRAMES; 
		
		// GPIOS
    PINMODE0 &= ~(3 << 20);
    FIO0DIR &= ~(RIGHT_BUTTON); 
    FIO3DIR |= (LEFT_GREEN | LEFT_RED | LEFT_BLUE | RIGHT_RED | RIGHT_GREEN | RIGHT_BLUE);
    PINSEL1 |=  (0x2 << 20);
		
		// Initialisations
    blow_bubbles();												// LCD
    touch_init();													// Touchscreen
    draw_ui();
		
    init_timer_polling();									// Timer
    hrs = 0; mins = 0; secs = 0;
    display_time(hrs, mins, secs);

    smartplug_init();											// Smartplug
    setupADC_Light();											// ADC for light sensor
    setup_DAC();													// DAC for speaker
		doorbell_init();											// Doorbell
  
    blind_1_timer_task(hrs, mins, secs);	// Blind 1 - Timer based
    blind_2_light_task();									// Blind 2 - Light sensor based
		
		// Loop
    while (1) {
				// Touchscreen
        char x=0, y=0, z1=0, z2=0;
        int pressure;
        int is_down;
				
				// Doorbell
        curr_btn_state = (FIO0PIN & RIGHT_BUTTON) ? 1 : 0;
				doorbell_poll();
				
				// Timer
				if (timer_update_seconds()) {
						unsigned int seconds = get_seconds();
						unsigned int hrs  = seconds / 3600;
						unsigned int mins = (seconds % 3600) / 60;
						unsigned int secs = seconds % 60;

						display_time(hrs, mins, secs);
						smartplug_update(hrs, mins, secs);
						blind_1_timer_task(hrs, mins, secs);
						blind_2_light_task();
				}

        //Touch handling 
				touch_read_xy(&x,&y, &z1, &z2);
				pressure = 16384*(z1) / (x*(z2-z1));
        is_down = (pressure >= 30);

        if (!is_down) {
            // count consecutive release frames; once enough, allow next press 
            if (release_frames < RELEASE_FRAMES) release_frames++;
            if (release_frames >= RELEASE_FRAMES) touch_ready = 1;
        } else {
            // finger is pressing rn
            release_frames = 0;  // reset release counter 

            if (touch_ready) {
                // Accept only one press per touch 
                check_light_button(x, y, &light_on);
                check_blind1_button(x, y, &blind_1_state);
                check_blind2_button(x, y, &blind_2_state);
                touch_ready = 0;  // block further presses until full release 
            }

        }

				smartplug_poll_button();

				light_sensor_display();
    }
}
