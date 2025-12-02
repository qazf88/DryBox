#include <bme280.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

#define READ_BME_TIME_MS 1000
#define READ_ERROR_TIMEOUT 3000

double BMEtemperature = 0;
double BMEhumidity = 0;
bool flag_BME_error = false;

Adafruit_BME280 bme;

bool getBMEError(){
    return flag_BME_error;
}

double getBMETemperatuere(){
    return BMEtemperature;
}

double getBMEHumidity(){
    return BMEhumidity;
}

void InitBME()
{
    bool status;
    status = bme.begin(0x76);
    if (!status)
    {
        while (1)
        {
            if (millis() > READ_ERROR_TIMEOUT)
            {
                flag_BME_error = true;
                break;
            }
        }
    }
}

void BMETask()
{
    static uint32_t time = 0;
    if (millis() - time > READ_BME_TIME_MS)
    {
        time = millis();
        BMEtemperature = bme.readTemperature();
        BMEhumidity = bme.readHumidity();

        // if (int(BMEhumidity) > 99 || int(BMEtemperature > 100))
        // {
        //     flag_BME_error = true;
        // }
    }
}