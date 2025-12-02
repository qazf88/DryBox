#ifndef LCD_H
#define LCD_H

#include <Arduino.h>

void InitLCD();
void showStatic(int set_temp, bool heater_enable, bool fan_enable, bool wifi_enable);
void showHeaterStatus(bool state);
void showFanStatus(bool state);
void showWIFIStatus(bool state);
void showIcon(bool isTemp);
void showMidle(float value, int ntc_value, bool error);
void showBottom(uint32_t timer, bool timer_state);

void drawMenuLine(const char *text, const char *val, bool exit);

#endif // LCD_H