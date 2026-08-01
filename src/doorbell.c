#include "lpc24xx.h"
#include "lcd/lcd_grph.h"
#include <stdint.h>
#include "doorbell.h"

/* externs provided elsewhere */
extern uint32_t timer_get_tick(void);       /* millisecond tick */
extern unsigned int doorbell_waveform[];    /* DAC sample table */
extern int get_doorbell_length(void);       /* length of table */

/* Define these here (not extern) so the linker has one definition */
volatile int playing = 0;                   /* global ON/OFF flag */
int doorbell_needs_redraw = 0;              /* UI flag */

/* Left button on P0.10 (active-low with pull-up) */
#define LEFT_BUTTON     (1u << 10)

/* UI message position */
#define MSG_X 10
#define MSG_Y 174

/* “Ding-dong” envelope (ms) */
#define DING_MS   180u
#define GAP_MS     80u
#define DONG_MS   260u

/* Wave playback control */
#define DAC_SHIFT  6

void doorbell(void)
{
    /* Simple segment state machine */
    enum { DB_IDLE = 0, DB_DING, DB_GAP, DB_DONG };
    static unsigned char state = DB_IDLE;

    static int initialized = 0;
    static int prev_button_state = 1;   /* 1=released, 0=pressed (active-low) */
    static int i = 0;                   /* waveform index */
    static unsigned int step = 1;       /* sample step (pitch) */
    static uint32_t seg_end_ms = 0;     /* when current segment ends */

    /* C90: declare locals before any statements */
    int curr_button_state = 1;
    uint32_t now = 0;

    /* --- one-time pin setup and UI clear --- */
    if (!initialized) {
        /* Ensure P0.10 is GPIO input with pull-up */
        PINSEL0  &= ~(3u << 20);    /* function = GPIO */
        FIO0DIR  &= ~LEFT_BUTTON;   /* input */
        PINMODE0 &= ~(3u << 20);    /* pull-up enabled */

        lcd_putString(MSG_X, MSG_Y, (unsigned char *)"                 ");
        prev_button_state = (FIO0PIN & LEFT_BUTTON) ? 1 : 0;
        initialized = 1;
        return;
    }

    /* Read inputs and time */
    curr_button_state = (FIO0PIN & LEFT_BUTTON) ? 1 : 0;
    now = timer_get_tick();

    /* Toggle behavior on falling edge (press): start if idle, else stop early */
    if (prev_button_state == 1 && curr_button_state == 0) {
        if (state == DB_IDLE) {
            /* Start DING */
            state = DB_DING;
            i = 0;
            step = 1; /* base pitch for DING */
            seg_end_ms = now + DING_MS;
            lcd_putString(MSG_X, MSG_Y, (unsigned char *)"Doorbell ringing");
            playing = 1;
        } else {
            /* Stop early */
            state = DB_IDLE;
            DACR = 0;
            lcd_putString(MSG_X, MSG_Y, (unsigned char *)"                 ");
            playing = 0;
            doorbell_needs_redraw = 1;
        }
    }
    prev_button_state = curr_button_state;

    /* Time-driven state advance (non-blocking) */
    if (state != DB_IDLE && (int32_t)(now - seg_end_ms) >= 0) {
        if (state == DB_DING) {
            state = DB_GAP;
            seg_end_ms = now + GAP_MS;
            DACR = 0;  /* mute during gap */
        } else if (state == DB_GAP) {
            state = DB_DONG;
            seg_end_ms = now + DONG_MS;
            step = 2;  /* change pitch for DONG (try 2 or 3) */
        } else { /* DB_DONG finished */
            state = DB_IDLE;
            DACR = 0;
            lcd_putString(MSG_X, MSG_Y, (unsigned char *)"                 ");
            playing = 0;
            doorbell_needs_redraw = 1;
        }
    }

    /* Feed DAC while ringing (DING or DONG) */
    if (state == DB_DING || state == DB_DONG) {
        DACR = doorbell_waveform[i] << DAC_SHIFT;

        /* Advance index with current step (controls pitch) */
        i += (int)step;
        if (i >= get_doorbell_length()) {
            i -= get_doorbell_length();
        }
    }
}
