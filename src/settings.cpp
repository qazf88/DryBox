#include <EEPROM.h>
#include <settings.h>

void Settings::saveTemp()
{
    if ((set_temp < SET_TEMP_MIN) || (set_temp > SET_TEMP_MAX))
    {
        set_temp = DEFAULT_SET_TEMP;
    }
    EEPROM.put(SET_TEMP_ADDRESS, int(set_temp));
}
void Settings::loadTemp()
{
    int t = DEFAULT_SET_TEMP;
    EEPROM.get(SET_TEMP_ADDRESS, t);
    if ((set_temp < SET_TEMP_MIN) || (set_temp > SET_TEMP_MAX))
    {
        set_temp = DEFAULT_SET_TEMP;
        EEPROM.put(SET_TEMP_ADDRESS, int(set_temp));
        nead_commit = true;
    }
    set_temp = t;
}

void Settings::saveTimerState()
{
    EEPROM.put(ENABLE_TIMER_ADDRESS, timer_state);
}
void Settings::loadTimerState()
{
    EEPROM.get(ENABLE_TIMER_ADDRESS, timer_state);
    if (timer_state < 0 || timer_state > 1)
    {
        timer_state = 0;
        EEPROM.put(ENABLE_TIMER_ADDRESS, timer_state);
        nead_commit = true;
    }
}

void Settings::saveTmerTime()
{
    uint8_t _hours = timer_time / 3600000;
    uint8_t _minutes = (timer_time % 3600000) / 60000;

    EEPROM.put(H_TIMER_ADDRESS, _hours);
    EEPROM.put(M_TIMER_ADDRESS, _minutes);
}
void Settings::loadTimerTime()
{
    uint8_t _hours = 0;
    uint8_t _minutes = 0;

    EEPROM.get(H_TIMER_ADDRESS, _hours);
    if (_hours > 24)
    {
        _hours = 1;
        EEPROM.put(H_TIMER_ADDRESS, _hours);
        nead_commit = true;
    }
    EEPROM.get(M_TIMER_ADDRESS, _minutes);
    if (_minutes > 60)
    {
        _minutes = 0;
        EEPROM.put(M_TIMER_ADDRESS, _minutes);
        nead_commit = true;
    }

    timer_time = (_hours * 3600000) + (_minutes * 60000);
}

void Settings::saveHeaterState()
{
    EEPROM.put(ENABLE_HEATER_ADDRESS, heater_state);
}
void Settings::loadHeaterState()
{
    EEPROM.get(ENABLE_HEATER_ADDRESS, heater_state);
    if (heater_state < 0 || heater_state > 1)
    {
        heater_state = false;
        EEPROM.put(ENABLE_HEATER_ADDRESS, heater_state);
        nead_commit = true;
    }
}

void Settings::saveFanState()
{
    EEPROM.put(ENABLE_FAN_ADDRESS, fan_state);
}
void Settings::loadFanState()
{
    EEPROM.get(ENABLE_FAN_ADDRESS, fan_state);
    if (fan_state < 0 || fan_state > 1)
    {
        fan_state = true;
        EEPROM.put(ENABLE_FAN_ADDRESS, fan_state);
        nead_commit = true;
    }
}

void Settings::saveWiFiState()
{
    EEPROM.put(ENABLE_WIFI_ADDRESS, wifi_state);
}
void Settings::loadWiFiState()
{
    EEPROM.get(ENABLE_WIFI_ADDRESS, wifi_state);
    if (wifi_state < 0 || wifi_state > 1)
    {
        wifi_state = true;
        EEPROM.put(ENABLE_WIFI_ADDRESS, wifi_state);
        nead_commit = true;
    }
}

void Settings::loadSettings()
{
    loadTemp();
    loadTimerState();
    loadTimerTime();
    loadHeaterState();
    loadFanState();
    loadWiFiState();

    if (nead_commit)
    {
        nead_commit = false;
        EEPROM.commit();
    }
}

void Settings::setTemp(double temp)
{
    set_temp = temp;
}
double Settings::getTemp()
{
    return set_temp;
}

void Settings::setTimerState(bool state)
{
    timer_state = state;
}
bool Settings::getTimerSate()
{
    return timer_state;
}

void Settings::setTimerTime(int hours, int minutes)
{
    timer_time = (hours * 3600000) + (minutes * 60000);
}
void Settings::setTimerTime(uint32_t milisecond)
{
    timer_time = milisecond;
}
uint32_t Settings::getTimerTime()
{
    return timer_time;
}

void Settings::setHeaterState(bool state)
{
    heater_state = state;
}
bool Settings::getHeaterState()
{
    return heater_state;
}

void Settings::setFanState(bool state)
{
    fan_state = state;
}
bool Settings::getFanState()
{
    return fan_state;
}

void Settings::setWiFiState(bool state)
{
    wifi_state = state;
}
bool Settings::getWiFiState()
{
    return wifi_state;
}

String Settings::getWiFiSSID()
{
    return wifi_ssid;
}

String Settings::getWiFiPass()
{
    return wifi_pass;
}

void Settings::saveSettings()
{
    Serial.println("save");
    saveTemp();
    saveTimerState();
    saveTmerTime();
    saveFanState();
    saveHeaterState();
    saveWiFiState();

    EEPROM.commit();
}

Settings::Settings()
{
    EEPROM.begin(EEPROM_SIZE);
    loadSettings();
}

Settings::~Settings()
{
}
