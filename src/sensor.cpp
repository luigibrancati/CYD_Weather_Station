#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "sensor.h"

void SensorData::init_sensor() {
  Serial.println("Init SHT31");
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  Serial.println("SHT31 found!");
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
}

void SensorData::read_sensor() {
  Serial.println("Reading temperature");
  float t = sht31.readTemperature();
  if (! isnan(t)) {  // check if 'is not a number'
    Serial.print("Temp *C = "); Serial.print(t); Serial.print("\t\t");
    this->temperature = t;
  } else {
    Serial.println("Failed to read temperature");
  }

  // Enable heater for 10 seconds, this helps get a better humidity reading
  // when the heater is enabled, the temperature increases by ~3.0 degC
  Serial.println("Enabling heater for 10 seconds");
  sht31.heater(true);
  delay(10000);
  sht31.heater(false);
  Serial.println("Heater disabled");
  Serial.println("Reading humidity");
  float h = sht31.readHumidity();
  if (! isnan(h)) {  // check if 'is not a number'
    Serial.print("Hum. % = "); Serial.println(h);
    this->humidity = h;
  } else {
    Serial.println("Failed to read humidity");
  }
}

void SensorData::print() const {
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}
