#ifndef SHT31_SENSOR_H
#define SHT31_SENSOR_H

#define I2C_SDA_PIN 27
#define I2C_SCL_PIN 22

#include "Adafruit_SHT31.h"

struct SensorData {
    float temperature;
    float humidity;
    Adafruit_SHT31 sht31;

    SensorData(): temperature(0.0f), humidity(0.0f), sht31(Adafruit_SHT31()) {};
    void init_sensor();
    void read_sensor();
    void print() const;

};

#endif // SHT31_SENSOR_H
