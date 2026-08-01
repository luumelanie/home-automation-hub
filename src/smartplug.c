#include "smartplug.h"
#include "lpc24xx.h"
#include "lcd/lcd_grph.h"

#define MODE_MANUAL_ON   1
#define MODE_MANUAL_OFF  2
#define RIGHT_BUTTON     (1 << 11) //P0.11

int coffee_on = 0;
int smartplug_mode = MODE_MANUAL_OFF;
int auto_on_triggered = 0;  // Tracks if auto ON has occurred so auto off cannot occur if auto on hasnt
static int last_btn_state = 1;

void smartplug_init(void) {
    smartplug_mode = MODE_MANUAL_OFF;
    coffee_on = 0;
    auto_on_triggered = 0;
		last_btn_state = (FIO0PIN & RIGHT_BUTTON) ? 1 : 0;
}

void smartplug_toggle_manual_override(void) {
    if (smartplug_mode == MODE_MANUAL_OFF) {
        smartplug_mode = MODE_MANUAL_ON;
        coffee_on = 0;
        auto_on_triggered = 0;  // Reset auto tracking when switching to manual ON
        lcd_putString(10, 200, (unsigned char *)"Coffee (manual ON )");
    }
    else if (smartplug_mode == MODE_MANUAL_ON) {
        smartplug_mode = MODE_MANUAL_OFF;
        coffee_on = 0;
        lcd_putString(10, 200, (unsigned char *)"Coffee (manual OFF)");
    }
}

void smartplug_update(unsigned int hrs, unsigned int mins, unsigned int secs) {
    if (smartplug_mode == MODE_MANUAL_ON) {
        if (!auto_on_triggered && hrs == 0 && mins == 0 && secs == 10) {
            coffee_on = 1;
            lcd_putString(10, 200, (unsigned char *)"Coffee (auto ON)   ");
            auto_on_triggered = 1;
        }
        else if (auto_on_triggered && hrs == 0 && mins == 0 && secs == 20) {
            coffee_on = 0;
            lcd_putString(10, 200, (unsigned char *)"Coffee (auto OFF)  ");
        }
    }
}

void smartplug_poll_button(void) {
    int curr_btn_state = (FIO0PIN & RIGHT_BUTTON) ? 1 : 0;
    if (curr_btn_state == 0 && last_btn_state == 1) {
        smartplug_toggle_manual_override();
    }
    last_btn_state = curr_btn_state;
}
