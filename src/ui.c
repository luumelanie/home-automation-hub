#include "lcd/lcd_grph.h"
#include "blinds.h"
#include "ui.h"

#define BTN_LIGHT_X     10
#define BTN_LIGHT_Y     40
#define BTN_LIGHT_W     130
#define BTN_LIGHT_H     26

#define BTN_BLIND1_X    BTN_LIGHT_X
#define BTN_BLIND1_Y    (BTN_LIGHT_Y + BTN_LIGHT_H + 12)
#define BTN_BLIND1_W    130
#define BTN_BLIND1_H    30

#define BTN_BLIND2_X    BTN_LIGHT_X
#define BTN_BLIND2_Y    (BTN_BLIND1_Y + BTN_BLIND1_H + 12)
#define BTN_BLIND2_W    130
#define BTN_BLIND2_H    30

#define TIMER_X 10
#define TIMER_Y 260
#define TIMER_W 200
#define TIMER_HEIGHT 60

// Touch press threshold and release debounce 
#define TOUCH_PRESS_THRESH 6
#define RELEASE_FRAMES      3   // require N consecutive not-pressed frames 

void draw_ui(void) {
    lcd_fillScreen(BLACK);
    lcd_fontColor(WHITE, BLACK);

    lcd_drawRect(BTN_LIGHT_X, BTN_LIGHT_Y,
                 BTN_LIGHT_X + BTN_LIGHT_W, BTN_LIGHT_Y + BTN_LIGHT_H, WHITE);
    lcd_putString(BTN_LIGHT_X + 10, BTN_LIGHT_Y + 5, (unsigned char*)"Light Button");

    lcd_drawRect(BTN_BLIND1_X, BTN_BLIND1_Y,
                 BTN_BLIND1_X + BTN_BLIND1_W, BTN_BLIND1_Y + BTN_BLIND1_H, WHITE);
    lcd_putString(BTN_BLIND1_X + 10, BTN_BLIND1_Y + 5, (unsigned char*)"Blind 1 Button");

    lcd_drawRect(BTN_BLIND2_X, BTN_BLIND2_Y,
                 BTN_BLIND2_X + BTN_BLIND2_W, BTN_BLIND2_Y + BTN_BLIND2_H, WHITE);
    lcd_putString(BTN_BLIND2_X + 10, BTN_BLIND2_Y + 5, (unsigned char*)"Blind 2 Button");

    lcd_putString(10, 170, (unsigned char*)"Light OFF");
    lcd_putString(10, 180, (unsigned char*)"Blind 1 ROLLED DOWN");
    lcd_putString(10, 190, (unsigned char*)"Blind 2 ROLLED DOWN");
    lcd_putString(10, 200, (unsigned char*)"Coffee (manual OFF)");

}

void check_light_button(char x, char y, int *light_on) {
    int sx = x * 240 / 255;
    int sy = y * 320 / 255;

    if (sx >= BTN_LIGHT_X && sx <= (BTN_LIGHT_X + BTN_LIGHT_W) &&
        sy >= BTN_LIGHT_Y && sy <= (BTN_LIGHT_Y + BTN_LIGHT_H)) {

        *light_on = !(*light_on);
        if (*light_on == 0) {
            lcd_putString(10, 170, (unsigned char*)"Light OFF (button pressed)           ");
        } else {
            lcd_putString(10, 170, (unsigned char*)"Light ON  (button pressed)           ");
        }
    }
}

void check_blind1_button(char x, char y, int *blind_1_state) { // left 
    int sx = x * 240 / 255;
    int sy = y * 320 / 255;
    if (sx >= BTN_BLIND1_X && sx <= (BTN_BLIND1_X + BTN_BLIND1_W) &&
        sy >= BTN_BLIND1_Y && sy <= (BTN_BLIND1_Y + BTN_BLIND1_H)) {
        blind_1_control();                 // DOWN->MID->UP->AUTO via timer 
        *blind_1_state = get_blind_1_state();
    }
}

void check_blind2_button(char x, char y, int *blind_2_state) { // right 
    int sx = x * 240 / 255;
    int sy = y * 320 / 255;
    if (sx >= BTN_BLIND2_X && sx <= (BTN_BLIND2_X + BTN_BLIND2_W) &&
        sy >= BTN_BLIND2_Y && sy <= (BTN_BLIND2_Y + BTN_BLIND2_H)) {
        blind_2_control();                 // DOWN->MID->UP->AUTO via light sensor
        *blind_2_state = get_blind_2_state();
    }
}
