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
static const String base_url = "https://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&timezone=" + timezone + "&forecast_days=3";
static String last_weather_update = "";
static lv_obj_t * text_label_time_location = nullptr;

struct Weather {
  String weather_description;
  lv_obj_t * weather_image;
  lv_obj_t * text_label_weather_description;

  Weather();
  virtual void update_fields(const JsonDocument&, int) = 0;
  void get_weather_description(bool, short);
};

// Store date and time
struct CurrentWeather : Weather {
  bool is_day;
  float apparent_temperature;
  float humidity;
  float wind_speed;
  short weather_code;
  lv_obj_t * text_label_apparent_temperature;
  lv_obj_t * text_label_humidity;
  lv_obj_t * text_label_wind_speed;

  CurrentWeather();
  CurrentWeather(const JsonDocument&);
  void update_fields(const JsonDocument&, int) override;
  void print() const;
};

// Store date and time
struct DailyWeather : Weather {
  short idx;
  bool is_rainy;
  float max_apparent_temperature;
  float min_apparent_temperature;
  short weather_code;
  String sunrise;
  String sunset;
  float max_wind_speed;
  float max_precipitation_probability;
  lv_obj_t * text_label_max_apparent_temperature;
  lv_obj_t * text_label_min_apparent_temperature;
  lv_obj_t * text_label_sunrise;
  lv_obj_t * text_label_sunset;
  lv_obj_t * text_label_max_wind_speed;
  lv_obj_t * text_label_max_precipitation_probability;

  DailyWeather();
  DailyWeather(const JsonDocument&, int);
  void update_fields(const JsonDocument&, int) override;
  void print() const;
};

JsonDocument get_weather_data();
String datetime_string_date(String);
String datetime_string_time(String);
String update_last_weather_update(const JsonDocument&);

#endif // WEATHER_DATA_H
