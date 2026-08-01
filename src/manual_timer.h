#ifndef MANUAL_TIMER_H
#define MANUAL_TIMER_H

extern int blinds_open_hour;
extern int blinds_half_hour;
extern int blinds_closed_hour;

void draw_timer_ui(void);
void handle_timer_touch(char x, char y);

#endif
