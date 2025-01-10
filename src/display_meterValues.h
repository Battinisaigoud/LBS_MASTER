#ifndef DISPLAY_METERVALUES_H
#define DISPLAY_METERVALUES_H

#include "Master.h"

void stateTimer();
void disp_dwin_meter();
void disp_lcd_meter();
void EVSE_Led_loop(void);

#if LCD_ENABLED
void cloud_no_rfid_lcd_print(void);
#endif
void cloud_no_rfid_dwin_print(void);

#endif