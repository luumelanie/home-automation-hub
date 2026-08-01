#include "blinds.h"
#include "lpc24xx.h"
#include "lcd/lcd_grph.h"
#include <stdio.h>
#include <stdint.h>
#include "light_sensor.h"   /* readADC_Light() */

/* Left (Blind 1) LEDs */
#define LEFT_RED        (1 << 16)
#define LEFT_GREEN      (1 << 17)
#define LEFT_BLUE       (1 << 18)

#define RIGHT_RED       (1 << 19)
#define RIGHT_GREEN     (1 << 20)
#define RIGHT_BLUE      (1 << 21)

/* Text baseline Y positions */
#define LINE_Y1 180
#define LINE_Y2 190
#define LINE_X  10

/* Clear helper: overwrite a whole line with spaces before reprinting */
static void clear_line(int y) {
    /* Long string of spaces to cover left text + tag area */
    static const unsigned char blanks[] =
        "                                                                                ";
    lcd_putString(LINE_X, y, (unsigned char *)blanks);
}

/* Light thresholds mV for sensor-based behavior, for blind 2 */
#define LIGHT_UP_MV_MAX        3000   //  > 3000 is rolled up
#define LIGHT_DOWN_MV_MIN      800    // < 1000 is rolled down
#define LIGHT_MID_MV_LOW       800    // else blinds at midway
#define LIGHT_MID_MV_HIGH      3000

int blind_1_state = STATE_ROLLED_DOWN;
int blind_2_state = STATE_ROLLED_DOWN;

typedef enum {
    MODE_MANUAL = 0,       // manual; state follows last button step 
    MODE_AUTO_TIMER = 1,   // Blind 1 auto by timer 
    MODE_AUTO_LIGHT = 2    // Blind 2 auto by light sensor
} blind_mode_t;

static blind_mode_t blind_1_mode = MODE_AUTO_TIMER; 
static blind_mode_t blind_2_mode = MODE_AUTO_LIGHT; 

static int last_blind_1_timer_action = -1;
static int last_blind_2_light_action = -1;

void blind_init(void) {
    // outputs 
    FIO3DIR |= (LEFT_RED | LEFT_GREEN | LEFT_BLUE |
                RIGHT_RED | RIGHT_GREEN | RIGHT_BLUE);

    // Start with both DOWN 
    blind_1_state = STATE_ROLLED_DOWN;
    blind_2_state = STATE_ROLLED_DOWN;

    FIO3SET = (LEFT_BLUE | RIGHT_BLUE);
    FIO3CLR = (LEFT_RED | LEFT_GREEN | RIGHT_RED | RIGHT_GREEN);

    // Start in AUTO 
    blind_1_mode = MODE_AUTO_TIMER;
    blind_2_mode = MODE_AUTO_LIGHT;
    last_blind_1_timer_action = -1;
    last_blind_2_light_action = -1;

    // Put status on LCD (clear then write)
    clear_line(LINE_Y1);
    lcd_putString(10, 180, (unsigned char *)"Blind 1 AUTO (timer)                 ");
    clear_line(LINE_Y2);
    lcd_putString(10, 190, (unsigned char *)"Blind 2 AUTO (light)                 ");
}

// Helpers
static void set_blind_1_led_and_msg(int state, const char* tag) {
    FIO3CLR = (LEFT_RED | LEFT_GREEN | LEFT_BLUE);
    clear_line(LINE_Y1);

    if (state == STATE_ROLLED_DOWN) {
        FIO3SET = LEFT_BLUE;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 ROLLED DOWN ");
        lcd_putString(170, 180, (unsigned char *)tag);
    } else if (state == STATE_MIDWAY) {
        FIO3SET = LEFT_GREEN;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 MIDWAY      ");
        lcd_putString(170, 180, (unsigned char *)tag);
    } else {
        FIO3SET = LEFT_RED;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 ROLLED UP   ");
        lcd_putString(170, 180, (unsigned char *)tag);
    }
}

static void set_blind_2_led_and_msg(int state, const char* tag) {
    FIO3CLR = (RIGHT_RED | RIGHT_GREEN | RIGHT_BLUE);
    clear_line(LINE_Y2);

    if (state == STATE_ROLLED_DOWN) {
        FIO3SET = RIGHT_BLUE;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 ROLLED DOWN");
        lcd_putString(170, 190, (unsigned char *)tag);
    } else if (state == STATE_MIDWAY) {
        FIO3SET = RIGHT_GREEN;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 MIDWAY");
        lcd_putString(170, 190, (unsigned char *)tag);
    } else {
        FIO3SET = RIGHT_RED;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 ROLLED UP");
        lcd_putString(170, 190, (unsigned char *)tag);
    }
}

// Button cycles with AUTO 
void blind_1_control(void) {
    if (blind_1_mode == MODE_AUTO_TIMER) {
        blind_1_mode  = MODE_MANUAL;
        blind_1_state = STATE_ROLLED_DOWN;
        set_blind_1_led_and_msg(blind_1_state, "(manual)");
        return;
    }

    // cycle: DOWN -> MID -> UP-> AUTO
    if (blind_1_state == STATE_ROLLED_DOWN) {
        blind_1_state = STATE_MIDWAY;
        set_blind_1_led_and_msg(blind_1_state, "(manual)");
    } else if (blind_1_state == STATE_MIDWAY) {
        blind_1_state = STATE_ROLLED_UP;
        set_blind_1_led_and_msg(blind_1_state, "(manual)");
    } else { 
        blind_1_mode = MODE_AUTO_TIMER;
        last_blind_1_timer_action = -1;
        clear_line(LINE_Y1);
        lcd_putString(10, 180, (unsigned char *)"Blind 1 AUTO (timer)");
    }
}

void blind_2_control(void) {
    if (blind_2_mode == MODE_AUTO_LIGHT) {
        blind_2_mode  = MODE_MANUAL;
        blind_2_state = STATE_ROLLED_DOWN;
        set_blind_2_led_and_msg(blind_2_state, "(manual)");
        return;
    }

    // cycle: DOWN -> MID -> UP-> AUTO
    if (blind_2_state == STATE_ROLLED_DOWN) {
        blind_2_state = STATE_MIDWAY;
        set_blind_2_led_and_msg(blind_2_state, "(manual)");
    } else if (blind_2_state == STATE_MIDWAY) {
        blind_2_state = STATE_ROLLED_UP;
        set_blind_2_led_and_msg(blind_2_state, "(manual)");
    } else { /* UP -> AUTO */
        blind_2_mode = MODE_AUTO_LIGHT;
        last_blind_2_light_action = -1;
        clear_line(LINE_Y2);
        lcd_putString(10, 190, (unsigned char *)"Blind 2 AUTO (light)                 ");
    }
}

// Automation task functions
void blind_1_timer_task(unsigned int hrs, unsigned int mins, unsigned int secs)
{
    int desired_state; 
    desired_state = -1;

    (void)hrs;   
    (void)mins;  

    if (blind_1_mode != MODE_AUTO_TIMER) return;

    // Change blinds state at specific times, (changed to 5, 10, 15, 20 seconds just 
    // for ease of testing since would have to wait too long for actual times of 6am, 7am, 6pm, 7:30pm)
    if (secs == 5) {
        desired_state = STATE_MIDWAY;
    } else if (secs == 10) {
        desired_state = STATE_ROLLED_UP;
    } else if (secs == 15) {
        desired_state = STATE_MIDWAY;
    } else if (secs == 20) {
        desired_state = STATE_ROLLED_DOWN;
    } else {
        desired_state = -1;
    }

    if (desired_state != -1 &&
        desired_state != blind_1_state &&
        desired_state != last_blind_1_timer_action) {
        set_blind_1_state(desired_state);
        last_blind_1_timer_action = desired_state;
    }
}

void blind_2_light_task(void) {
    unsigned int code;     
    unsigned int mv;
    int desired_state;

    code = 0;
    mv   = 0;
    desired_state = -1;

    if (blind_2_mode != MODE_AUTO_LIGHT) return;

    code = (unsigned int)readADC_Light();   // 0-1023
    mv   = (code * 3300) / 1023;            // 0-3.3V

    if (mv >= LIGHT_UP_MV_MAX) {
        desired_state = STATE_ROLLED_UP;
    } else if (mv < LIGHT_DOWN_MV_MIN) {
        desired_state = STATE_ROLLED_DOWN;
    } else if (mv >= LIGHT_DOWN_MV_MIN && mv <= LIGHT_UP_MV_MAX) {
        desired_state = STATE_MIDWAY;
    } else if (mv > LIGHT_UP_MV_MAX) {
        desired_state = STATE_ROLLED_UP;    
    } else {
        /* here: 100..399 mV 
           desired_state = STATE_ROLLED_DOWN;  */
    }

    if (desired_state != -1 &&
        desired_state != blind_2_state &&
        desired_state != last_blind_2_light_action) {
        set_blind_2_state(desired_state);
        last_blind_2_light_action = desired_state;
    }
}

// Setters used by automation 
void set_blind_1_state(int state) {
    if (state < STATE_ROLLED_DOWN || state > STATE_ROLLED_UP) return;

    blind_1_state = state;
    FIO3CLR = (LEFT_RED | LEFT_GREEN | LEFT_BLUE);
    clear_line(LINE_Y1);

    if (state == STATE_ROLLED_DOWN) {
        FIO3SET = LEFT_BLUE;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 ROLLED DOWN (timer)          ");
    } else if (state == STATE_MIDWAY) {
        FIO3SET = LEFT_GREEN;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 MIDWAY (timer)               ");
    } else {
        FIO3SET = LEFT_RED;
        lcd_putString(10, 180, (unsigned char *)"Blind 1 ROLLED UP (timer)            ");
    }
}

void set_blind_2_state(int state) {
    if (state < STATE_ROLLED_DOWN || state > STATE_ROLLED_UP) return;

    blind_2_state = state;
    FIO3CLR = (RIGHT_RED | RIGHT_GREEN | RIGHT_BLUE);
    clear_line(LINE_Y2);

    if (state == STATE_ROLLED_DOWN) {
        FIO3SET = RIGHT_BLUE;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 ROLLED DOWN (light)          ");
    } else if (state == STATE_MIDWAY) {
        FIO3SET = RIGHT_GREEN;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 MIDWAY (light)               ");
    } else {
        FIO3SET = RIGHT_RED;
        lcd_putString(10, 190, (unsigned char *)"Blind 2 ROLLED UP (light)            ");
    }
}

int get_blind_1_state(void) { return blind_1_state; }
int get_blind_2_state(void) { return blind_2_state; }
