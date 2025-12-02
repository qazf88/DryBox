#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>

#define EEPROM_SIZE 10

#define SET_TEMP_ADDRESS 0
#define ENABLE_TIMER_ADDRESS 4
#define H_TIMER_ADDRESS 5
#define M_TIMER_ADDRESS 6
#define ENABLE_HEATER_ADDRESS 7
#define ENABLE_FAN_ADDRESS 8
#define ENABLE_WIFI_ADDRESS 9
#define WIFI_LOGIN_ADDRESS 20
#define WIFI_PASS_ADDRESS 30 // +10

#define DEFAULT_SET_TEMP 40.0
#define SET_TEMP_MAX 60
#define SET_TEMP_MIN 30

#define TIME_SLEAP_DEFAULT 60 * 60 * 1000

class Settings
{
private:
    double set_temp = DEFAULT_SET_TEMP;
    bool timer_state = false;
    uint32_t timer_time = 0;
    bool heater_state = false;
    bool fan_state = true;
    bool wifi_state = true;
    String wifi_ssid = "qazf";
    String wifi_pass = "qazfqazf";

    bool nead_commit = false;
    void saveTemp();
    void loadTemp();
    void saveTimerState();
    void loadTimerState();
    void saveTmerTime();
    void loadTimerTime();
    void saveHeaterState();
    void loadHeaterState();
    void saveWiFiState();
    void loadWiFiState();
    void saveFanState();
    void loadFanState();

public:
    void setTemp(double temp);
    double getTemp();
    void setTimerState(bool state);
    bool getTimerSate();
    void setTimerTime(int hours, int minutes);
    void setTimerTime(uint32_t milisecond);
    uint32_t getTimerTime();
    void setHeaterState(bool state);
    bool getHeaterState();
    void setFanState(bool state);
    bool getFanState();
    void setWiFiState(bool state);
    bool getWiFiState();
    String getWiFiSSID();
    String getWiFiPass();
    void saveSettings();
    void loadSettings();

    Settings();
    ~Settings();
};

#endif // SETTINGS_H