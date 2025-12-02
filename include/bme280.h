#ifndef BME_280_H
#define BME_280_H

bool getBMEError();
void InitBME();
void BMETask();
double getBMETemperatuere();
double getBMEHumidity();

#endif // BME_280_H