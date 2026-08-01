// smartplug.c
#ifndef SMARTPLUG_H
#define SMARTPLUG_H

void smartplug_init(void);
void smartplug_update(unsigned int hrs, unsigned int mins, unsigned int secs);
void smartplug_toggle_manual_override(void);
void smartplug_poll_button(void);

extern int coffee_on;
extern int manual_override;

#endif
