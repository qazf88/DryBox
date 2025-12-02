#include <ntc.h>
#include <Arduino.h>

#define NTC_PIN 35
#define NTC_READ_TIME 500

const double VCC = 3.3;
const double R_FIXED = 100000.0;
const double BETA = 3950.0;
const double Temp0 = 298.15;
const double R0 = 100000.0;

double temperature = 0.0;

double getHeaterTemperature(){
    return temperature;
}

void NTCTask()
{
    uint32_t time = 0;

    if ((millis() - time) > NTC_READ_TIME)
    {

        time = millis();

        uint32_t summ = 0;
        for (int i = 0; i < 10; i++)
        {
            summ += analogRead(NTC_PIN);
        }

        int adc = summ / 10;
        double Vout = (adc / 4095.0) * VCC;
        double Rntc = R_FIXED * (Vout / (VCC - Vout));

        double tempK = 1.0 / (1.0 / Temp0 + (1.0 / BETA) * log(Rntc / R0));
        double tempC = tempK - 273.15;
        temperature = tempC;
    }
}