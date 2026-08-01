#include "lpc24xx.h"
#include "doorbell.h"
#include "play_tone.h"
#include "lcd/lcd_grph.h"
#include <stdint.h>
#include "songs.h"
#include "lpc24xx.h"

// P0.10 
#define DOORBELL_BTN   (1 << 10)

// Debounce timings
#define DB_PRESS_MS    25
#define DB_RELEASE_MS  25

// Doorbell song
extern uint32_t timer_get_tick(void);
extern struct tone song_data[];
extern int song_duration;
extern void udelay(unsigned int delay_in_us);
extern void play_tone(unsigned int duration, int period,  int vol);
extern void setup_DAC(void);

// States
static uint8_t  stable_state = 1;       // 1 = released and 0 = pressed
static uint8_t  last_reported = 1;
static uint32_t last_change_ms = 0;
static uint8_t  armed = 1;

static __inline uint8_t raw_btn(void) {
    return (FIO0PIN & DOORBELL_BTN) ? 1 : 0;
}


void doorbell_init(void)
{
    uint32_t now;

    PINSEL0  &= ~(3 << 20);    // P0.10 function = GPIO 
    FIO0DIR  &= ~DOORBELL_BTN;  //input 
    PINMODE0 &= ~(3 << 20);    // pull-up enabled 

    now = timer_get_tick();
    stable_state   = raw_btn();
    last_reported  = stable_state;
    last_change_ms = now;
    armed          = 1u;
}

void doorbell_poll(void)
{
		int rate = 52000; 
		int i = 0;

    uint32_t now;
    uint8_t  r;
    uint32_t dwell;

    now = timer_get_tick();
    r   = raw_btn();
    if (r != stable_state) {
        last_change_ms = now;
        stable_state   = r;
    } else {
        dwell = now - last_change_ms;
        if ((stable_state == 0u && dwell >= DB_PRESS_MS) ||
            (stable_state == 1u && dwell >= DB_RELEASE_MS)) {

            if (stable_state != last_reported) {
                last_reported = stable_state;

                // Press (active-low)
                if (stable_state == 0u && armed) {
                    // show message 
                    lcd_putString(10, 210, (unsigned char *)"Doorbell ringing");

										// Play the song
										while(i < 20) {										
											  // Play the note
												play_tone(rate * song_data[i].duration, song_data[i].pitch, song_data[i].volume);
												i++;
											}

                    // Clear message immediately after sound finishes 
                    lcd_putString(10, 210, (unsigned char *)"                 ");

                    // Disarm until released 
                    armed = 0u;
                }

                //Re-arm on release 
                if (stable_state == 1u) {
                    armed = 1u;
                }
            }
        }
    }
}
