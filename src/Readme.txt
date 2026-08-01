.txt file in your design directory outlining the system
features and architecture. 


Source code folder: 
Contains:
1) main.c 

Initialisation and function of each smarthome feature including: 
- light sensor
- blinds
- doorbell
- smartplug
- LCD display and touchscreen

2) delay.c
- Contained mdelay function used in lcd_hw, sdram

3) bubbles.c

Functions contained: 
- void blow_bubbles(void) : Used to initialise the LCD display, kept exactly the same from lab 6

4) touch.c
Functions kept the same from lab 6

Functions contained :
- void touch_init : used to initialise the touchscreen
- void touch_read_xy : used to get x and y coordinates for touchscreen operation, and z1 and z1 for pressure calculation in main
- static unsigned char touch_read : Used to process touch coordinates

5) smartplug.c

Contains smartplug implementation. 

How it works: 
- Has a manual button P0.11 which toggles on/of
- If it is in the on state, then auto mode will trigger at 10 seconds to heat up coffee and off at 20 seconds to save on energy bills 
- If it is in the off state, then auto mode will not trigger at all

Functions contained: 
- void smartplug_init(void) : Initializes smart plug state to manual OFF, resets auto mode flags, and reads the initial right button state.
- void smartplug_toggle_manual_override(void) : Toggles the smart plug between manual ON and manual OFF modes, updating the LCD with the new status.
- void smartplug_update(unsigned int hrs, unsigned int mins, unsigned int secs): In manual ON mode, automatically turns the coffee machine ON at 10 seconds and OFF at 20 seconds, 
																				updating the LCD accordingly. Uses auto_on_triggered to ensure OFF only happens if ON has occurred.
- void smartplug_poll_button(void): Monitors the right button (P0.11) for presses and calls smartplug_toggle_manual_override() 

6) blinds.c

Contains blinds implementation.

How it works:
The LED above P0.10 is the LEFT LED and the LED above P0.11 is the RIGHT LED.
The LED colours indicate the blind states as follows:
- red = rolled up
- green = midway
- blue = rolled down

LEFT LED:
The LEFT LED states can be toggled by pessing the BLIND 1 BUTTON. The order is DOWN -> MID -> UP-> AUTO. This cycle is repeated once finished.
When the LEFT LED is in AUTO mode, indicated by the status texts on below the buttons, it is automatically updated based on a timer.
The schedule is: 
- 00:00:05 = midway
- 00:00:10 = rolled up
- 00:00:15 = midway
- 00:00:20 = rolled down
These times are meant as a replacement  for ease of testing since would have to wait too long for actual times of 6am, 7am, 6pm, 7:30pm

If the user manually changes blind1, the auto mode will not activate until the user toggles back to AUTO mode.
Upon initialisation, the LEFT LED (Blind 1) is in auto mode until the user manually changes the state.

RIGHT LED:
The RIGHT LED states can be toggled by pessing the BLIND 2 BUTTON. The order is DOWN -> MID -> UP-> AUTO. This cycle is repeated once finished.
When the RIGHT LED is in AUTO mode, indicated by the status texts on below the buttons, it is automatically updated based on light sensor values.
The states based on the light sensor values in mV are as follows: 
- if mV readings are >= 3000, the blinds are rolled up
- if mV readings are <= 800,  the blinds are rolled down
- if 800 =< mV <= 3000, the blinds are midway

To get light levels up to 3000, testing can be done using a phone flashlight.

If the user manually changes blind2, the auto mode will not activate until the user toggles back to AUTO mode.
Upon initialisation, the RIGHT LED (Blind2) is in auto mode until the user manually changes the state.

Allows users to save energy bills by maximising natural light and heating. 

Functions contained:
- void blind_init(void): Configures LED pins for both blinds, sets initial states to rolled down, and initializes each blind to its default automation mode (Blind 1: timer-based, Blind 2: light-sensor-based).
- static void set_blind_1_led_and_msg(int state, const char* tag): Updates Blind 1’s LED colour and LCD text according to its state
- static void set_blind_2_led_and_msg(int state, const char* tag): Same as above, but for Blind 2.
- void blind_1_control(void): Manual control function for Blind 1, cycling through rolled down -> midway -> rolled up -> auto mode, updating LEDs and LCD accordingly.
- void blind_2_control(void): Manual control function for Blind 2, cycling through rolled down -> midway -> rolled up -> auto mode in the same way.
- void blind_1_timer_task(unsigned int hrs, unsigned int mins, unsigned int secs): In timer auto mode, changes Blind 1’s position at specific times (modified for testing: 5s, 10s, 15s, 20s).
- void blind_2_light_task(void): In light sensor auto mode, reads the TEMT6000 light sensor via readADC_Light() and adjusts Blind 2’s position based on measured voltage thresholds. Uses last_blind_2_light_action to prevent unnecessary updates.
- void set_blind_1_state(int state): Sets Blind 1’s state in auto mode and updates its LED and LCD text (includes mode tag).
- void set_blind_2_state(int state): Same as above, but for Blind 2.
- int get_blind_1_state(void): returns current state of blind1
- int get_blind_2_state(void): returns current state of blind2

7) timer.c

Contains function to display timer on LCD

Functions contained: 
- void display_time(unsigned int hrs, unsigned int mins, unsigned int secs): Formats the current time as HH:MM:SS and displays it on the LCD at coordinates (10, 30).

8) timer_asm.c

Implements l timer control and timekeeping in ARM assembly for precise tracking using Timer0 and replaces the original polling-based commented out in C implementation from timer.c.

Functions contained:
- init_timer_polling : Configures and starts Timer0 in polling mode 
- timer_update_seconds : Checks if 1000 ms have passed since the last update and if so, increments the seconds counter and updates last_tick. Returns 1 when a second has elapsed, otherwise returns 0.
- get_seconds : Returns the current value of the seconds
- timer_get_tick : Returns the current tick count from Timer0

9) doorbell_new.c

Contains doorbell initialisation and functions

How it works: 
Doorbell is implemented on P0.10. When pressed, it will play a doorbell sound and notify users on the LCD screen that the doorbell is ringing.
It uses the nyan cat song used in lab 5, and files used to play the song can be found in the Doorbell Song folder

Functions contained: 
- static __inline uint8_t raw_btn(void): Helper function that reads P0.10 directly (FIO0PIN & DOORBELL_BTN) and returns 1 if the button is released  or 0 if pressed.
- void doorbell_init(void): Configures P0.10 as a GPIO input with pull-up enabled for the doorbell button, reads its initial state, and sets up debouncing variables
								- DAC initialisation is in the file play_tone.c in the Doorbell Song Folder
- void doorbell_poll(void): Continuously checks the doorbell button with debounce timing (DB_PRESS_MS / DB_RELEASE_MS), detects presses/releases, and triggers actions on a valid press

10) light_sensor.c

Contains ADC initialisation and functions required to convert and display light levels on LCD.

How it works: 
Light sensor values are displayed on LCD from ADC readings (0-1023).

Functions contained: 
- void setupADC_Light(void): Powers up ADC0, configures its peripheral clock (PCLK = CCLK/4), and sets P0.24 to the AD0.1 function for light sensor input.
- int readADC_Light(void): Reads a single 10-bit ADC sample from channel AD0.1
- void light_sensor_display(void): Calls readADC_Light(), converts the reading to millivolts, and prints both raw ADC code and millivolt value to the LCD

11) ui.c

Handles the LCD-based user interface, including drawing the control buttons for the light, blinds, and coffee machine, and detecting touchscreen presses to trigger corresponding actions.

Functions contained:
- void draw_ui(void): Clears the LCD to black, draws button outlines for the light, Blind 1, and Blind 2 at fixed positions, and displays the default status text for each system component.
- void check_light_button(char x, char y, int *light_on): Converts raw touchscreen X/Y values to pixel coordinates, checks if the light button was pressed, toggles light_on, and updates the LCD
- void check_blind1_button(char x, char y, int *blind_1_state): Converts touchscreen coordinates, checks if Blind 1’s button was pressed, calls blind_1_control() to cycle through modes, and updates blind_1_state with get_blind_1_state().
- void check_blind2_button(char x, char y, int *blind_2_state): Same as above

//////////////////////////////////////////////////////////////////////////
LCD Folder
1) font5x.7.c : Text functions to render characters on screen.
2) lcd_graph.c : Implements high-level LCD graphics functions
3) lcd_hw.c : LCD hardware driver code that directly interfaces with the LCD
4) sdram.c : Initializes and configures the external SDRAM used as a framebuffer for the LCD

//////////////////////////////////////////////////////////////////////////
Doorbell song folder
1) songs.c : Defines the song_data[] array containing the sequence of notes (pitch, duration, volume) and song duration
2) play_tone.c : Implements DAC-based audio tone generation functions (play_tone(), setup_DAC())