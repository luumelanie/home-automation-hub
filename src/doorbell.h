#ifndef DOORBELL_H
#define DOORBELL_H

void init_lcd(void);
void doorbell(void);
extern int doorbell_needs_redraw;
extern volatile int playing;

#endif
