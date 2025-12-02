#include <lcd.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <User_Setup.h>
#include <icons.h>

TFT_eSPI tft = TFT_eSPI();

bool flag_menu = false;
int menuState = 0;

void InitLCD()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.fillRect(0, 0, 240, 40, TFT_WHITE);
  tft.fillRect(0, 200, 240, 40, TFT_WHITE);
}

void showSetTemp(int temp)
{
  tft.fillRect(0, 0, 60, 40, TFT_WHITE);
  tft.setCursor(15, 9);
  tft.setTextFont(1);
  tft.setTextSize(3);
  tft.setTextColor(TFT_BLACK);
  tft.print(temp);
  tft.setTextSize(1);
}

void showHeaterStatus(bool state)
{
  tft.fillCircle(80, 20, 8, state ? TFT_RED : TFT_WHITE);
}

void showFanStatus(bool state)
{
  tft.fillCircle(125, 20, 8, state ? TFT_YELLOW : TFT_WHITE);
}

void showWIFIStatus(bool state)
{
  tft.fillCircle(170, 20, 8, state ? TFT_GREEN : TFT_WHITE);
}

void showIcon(bool isTemp)
{
  if (flag_menu)
  {
    tft.fillRect(200, 0, 40, 40, TFT_WHITE);
  }
  else
  {
    tft.pushImage(210, 5, 25, 50, (isTemp ? iconTemp : iconHum));
  }
}

void showStatic(int set_temp, bool heater_enable, bool fan_enable, bool wifi_enable)
{
  showSetTemp(set_temp);
  tft.drawCircle(80, 20, 11, heater_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(125, 20, 11, fan_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(170, 20, 11, wifi_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(80, 20, 10, heater_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(125, 20, 10, fan_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(170, 20, 10, wifi_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(80, 20, 9, heater_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(125, 20, 9, fan_enable ? TFT_BLACK : TFT_WHITE);
  tft.drawCircle(170, 20, 9, wifi_enable ? TFT_BLACK : TFT_WHITE);
}

void showMidle(float value, int ntc_value, bool error)
{

  if (flag_menu)
  {
    return;
  }

  tft.fillRect(0, 40, 240, 160, TFT_BLACK);

  if (error)
  {
    tft.setTextDatum(MC_DATUM);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(6);
    tft.drawString("Error!", 130, 115, 1);
    tft.setTextSize(1);
  }
  else
  {
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", value);

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(buf, 120, 110, 8);
  }

  tft.setTextSize(2);
  tft.setCursor(200, 180);
  tft.printf("%d", ntc_value);
}

void showBottom(uint32_t timer, bool timer_state)
{

  if (timer_state)
  {
    uint32_t _t_sleap = timer / 1000;
    int minutes = (_t_sleap % 3600) / 60;
    int hours = _t_sleap / 3600;
    tft.setTextSize(3);
    tft.setCursor(20, 210);
    tft.setTextColor(TFT_BLACK);
    tft.fillRect(0, 200, 120, 40, TFT_WHITE);
    tft.printf("%02d:%02d", hours, minutes);
  }
  else
  {
    tft.setTextSize(2);
    tft.setCursor(10, 215);
    tft.setTextColor(TFT_BLACK);
    tft.fillRect(0, 200, 120, 40, TFT_WHITE);
    tft.print("Timer OFF");
  }

  int minutes = ((millis() / 1000) % 3600) / 60;
  int hours = millis() / 1000 / 3600;
  tft.setTextSize(3);
  tft.setCursor(135, 210);
  tft.setTextColor(TFT_BLACK);
  tft.fillRect(120, 200, 120, 40, TFT_WHITE);
  tft.printf("%02d:%02d", hours, minutes);

  tft.setTextSize(1);
}

void drawMenuLine(const char *text, const char *val, bool exit)
{
  if (exit)
  {
    flag_menu = false;
  }
  else
  {
    flag_menu = true;
  }

  tft.fillRect(0, 40, 240, 160, TFT_BLACK);
  tft.setTextFont(1);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(text, 120, 80, 1);
  tft.setTextSize(4);
  tft.drawString(val, 120, 145, 1);
  tft.setTextSize(1);
}
