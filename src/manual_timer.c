#include "manual_timer.h"
#include "lcd/lcd_grph.h"
#include <stdio.h>

int blinds_open_hour = 19;
int blinds_half_hour = 12;
int blinds_closed_hour = 7;

#define TIMER_X 10
#define TIMER_Y 230
#define TIMER_W 200
#define TIMER_H 60

void draw_timer_ui(void) {
		char buf[20];
    lcd_drawRect(TIMER_X, TIMER_Y, TIMER_X + TIMER_W, TIMER_Y + TIMER_H, WHITE);
    lcd_putString(TIMER_X + 5, TIMER_Y + 5, "Blinds Timer:");
    
    sprintf(buf, "Open : %02d:00", blinds_open_hour);
    lcd_putString(TIMER_X + 5, TIMER_Y + 20, (unsigned char *)buf);

		sprintf(buf, "Half: %02d:00", blinds_half_hour);
    lcd_putString(TIMER_X + 5, TIMER_Y + 35, (unsigned char *)buf);
	
    sprintf(buf, "Close: %02d:00", blinds_closed_hour);
    lcd_putString(TIMER_X + 5, TIMER_Y + 50, (unsigned char *)buf);
}

void handle_timer_touch(char x, char y) {
    int sx = x * 240 / 255;
    int sy = y * 320 / 255;

    if (sx >= TIMER_X && sx <= TIMER_X + TIMER_W &&
        sy >= TIMER_Y && sy <= TIMER_Y + TIMER_H) {
        
        int relative = sy - TIMER_Y;

        if (relative < 25) { // first row
            blinds_open_hour = (blinds_open_hour + 1) % 24;
        } else if (relative < 45) { // second row
            blinds_half_hour = (blinds_half_hour + 1) % 24;
        } else { // third row
            blinds_closed_hour = (blinds_closed_hour + 1) % 24;
        }

        draw_timer_ui();
    }
}
