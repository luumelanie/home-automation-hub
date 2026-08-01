#ifndef BLINDS_H
#define BLINDS_H

#define STATE_ROLLED_DOWN   0
#define STATE_MIDWAY        1
#define STATE_ROLLED_UP     2

void blind_init(void);          
void blind_1_control(void);     
void blind_2_control(void);     
void blind_task(void);          
void blind_2_timer_task(unsigned int hrs, unsigned int mins); 
unsigned int read_light_sensor(void);
int get_blind_1_state(void);    
int get_blind_2_state(void);    
void set_blind_1_state(int state);
void set_blind_2_state(int state);

#endif
