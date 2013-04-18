/*
 * Toplevel.h
 *
 * Created: 3/16/2012 4:52:12 PM
 *  Author: mtwiegx1
 */ 


char* get_LCD_cmd(void);

bool detect_lcd(void);

uint8_t idle_getmode(void);

char* get_HOST_cmd(void);

uint8_t check_SD(void);

bool SD_card_inserted(void);

void idle(void);

uint8_t hostmode_run(void);

bool standalone_mode_run(void);

void get_experiment_from_host_to_SD(void);

bool wait_for_CR(void);