#ifndef WEATHER_DATA_H
#define WEATHER_DATA_H

#include <lvgl.h>
#include <ArduinoJson.h>

static const char* ssid = "FASTWEB-FC83A5";
static const char* password = "MERHGZ6ECG";
static const char degree_symbol[] = "\u00B0C";
static const String latitude = "45.07049768682";
static const String longitude = "7.68682";
static const String timezone = "Europe/Rome";
static const String location = "Turin";

struct Weather {
  String weather_description;
  lv_obj_t * weather_image;
  lv_obj_t * text_label_weather_description;

  Weather();
  virtual void update_fields(const JsonDocument&) = 0;
  void get_weather_description(bool, int);
};

// Store date and time
struct CurrentWeather : Weather {
  String last_weather_update;
  bool is_day;
  float temperature;
  float humidity;
  short weather_code;
  float apparent_temperature;
  lv_obj_t * text_label_date;
  lv_obj_t * text_label_temperature;
  lv_obj_t * text_label_humidity;
  lv_obj_t * text_label_time_location;

  CurrentWeather();
  CurrentWeather(const JsonDocument&);
  void update_fields(const JsonDocument&) override;
  const String last_update_day() const;
  const String last_update_time() const;
  void print() const;
};

// Store date and time
struct ForecastWeather : Weather {
  bool is_rainy;
  int max_weather_code;
  float max_apparent_temperature;
  float min_apparent_temperature;
  lv_obj_t * text_label_max_temperature;
  lv_obj_t * text_label_min_temperature;

  ForecastWeather();
  ForecastWeather(const JsonDocument&);
  void update_fields(const JsonDocument&);
  void print() const;
};

JsonDocument get_weather_data();
void update_data(lv_timer_t*);

#endif // WEATHER_DATA_H
