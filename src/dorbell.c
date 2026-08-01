#include "doorbell.h"
#include "lpc24xx.h"
#include "lcd/lcd_hw.h"
#include "lcd/lcd_grph.h"
#include "lcd/lcd_cfg.h"
#include "lcd/sdram.h"
#include "delay.h"
#include "touch.h"

extern unsigned int doorbell_waveform[];
extern int get_doorbell_length(void);
volatile int playing = 0;

// LCD UI constants
#define BTN_X 60
#define BTN_Y 180
#define BTN_W 160
#define BTN_H 40

// State to avoid redrawing
static int button_drawn = 0;
static int prev_button_state = 1;
static int i = 0;

// Initialize LCD
void init_lcd(void) {
    sdramInit();
    lcdInit(&lcd_config);
    lcdTurnOn();
}

// Check if touch input hits the button, and stop playback
static void check_stop_button_touch(char x, char y) {
    int point_x = x * 240 / 255;
    int point_y = y * 320 / 255;

    if (point_x >= BTN_X && point_x <= (BTN_X + BTN_W) &&
        point_y >= BTN_Y && point_y <= (BTN_Y + BTN_H)) {
        playing = 0;
    }
}

// One call to run doorbell behavior
void doorbell(void) {
    char x = 0, y = 0;

    // --- Toggle doorbell on physical button press ---
    int curr_button_state = (FIO0PIN & (1 << 10)) ? 1 : 0;
    if (prev_button_state == 1 && curr_button_state == 0) {
        playing = !playing;
    }
    prev_button_state = curr_button_state;

    // --- Check touchscreen ---
    touch_read_xy(&x, &y);
    if (touch_get_pressure() > 1) {
        check_stop_button_touch(x, y);
    }

    // --- LCD display update ---
    lcd_fontColor(WHITE, BLACK);
    if (playing) {
        lcd_fillRect(0, 100, 320, 130, BLACK);
        lcd_putString(60, 100, "Doorbell ringing");

        if (!button_drawn) {
            lcd_drawRect(BTN_X, BTN_Y, BTN_X + BTN_W, BTN_Y + BTN_H, WHITE);
            lcd_putString(BTN_X + 10, BTN_Y + 10, "Stop Doorbell");
            button_drawn = 1;
        }
    } else {
        lcd_fillRect(0, 100, 320, 130, BLACK);
        if (button_drawn) {
            lcd_fillRect(BTN_X, BTN_Y, BTN_X + BTN_W, BTN_Y + BTN_H, BLACK);
            button_drawn = 0;
        }
    }

    // --- Output waveform if playing ---
    if (playing) {
        DACR = doorbell_waveform[i] << 6;
        i = (i + 1) % get_doorbell_length();
    } else {
        DACR = 0;
        i = 0;
    }
}
