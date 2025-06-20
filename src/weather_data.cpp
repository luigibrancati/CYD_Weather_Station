#include "weather_data.h"
#include "weather_images.h"
#include <WiFi.h>
#include <HTTPClient.h>

Weather::Weather(){
  weather_image = nullptr;
  text_label_weather_description = nullptr;
};

void Weather::get_weather_description(bool is_day, short weather_code) {
  Serial.print("Get weather description for code ");
  Serial.println(weather_code);
  switch (weather_code) {
    case 0:
      if(is_day) {
        lv_image_set_src(this->weather_image, &image_weather_sun); }
      else { lv_image_set_src(this->weather_image, &image_weather_night); }
      this->weather_description = "CLEAR SKY";
      break;
    case 1: 
      if(is_day) { lv_image_set_src(this->weather_image, &image_weather_sun); }
      else { lv_image_set_src(this->weather_image, &image_weather_night); }
      this->weather_description = "MAINLY CLEAR";
      break;
    case 2: 
      lv_image_set_src(this->weather_image, &image_weather_cloud);
      this->weather_description = "PARTLY CLOUDY";
      break;
    case 3:
      Serial.println("Weather code 3");
      lv_image_set_src(this->weather_image, &image_weather_cloud);
      Serial.println("Set weather image to cloud");
      this->weather_description = "OVERCAST";
      break;
    case 45:
      lv_image_set_src(this->weather_image, &image_weather_cloud);
      this->weather_description = "FOG";
      break;
    case 48:
      lv_image_set_src(this->weather_image, &image_weather_cloud);
      this->weather_description = "DEPOSITING RIME FOG";
      break;
    case 51:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "DRIZZLE LIGHT INTENSITY";
      break;
    case 53:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "DRIZZLE MODERATE INTENSITY";
      break;
    case 55:
      lv_image_set_src(this->weather_image, &image_weather_rain); 
      this->weather_description = "DRIZZLE DENSE INTENSITY";
      break;
    case 56:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "FREEZING DRIZZLE LIGHT";
      break;
    case 57:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "FREEZING DRIZZLE DENSE";
      break;
    case 61:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN SLIGHT INTENSITY";
      break;
    case 63:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN MODERATE INTENSITY";
      break;
    case 65:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN HEAVY INTENSITY";
      break;
    case 66:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "FREEZING RAIN LIGHT INTENSITY";
      break;
    case 67:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "FREEZING RAIN HEAVY INTENSITY";
      break;
    case 71:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW FALL SLIGHT INTENSITY";
      break;
    case 73:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW FALL MODERATE INTENSITY";
      break;
    case 75:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW FALL HEAVY INTENSITY";
      break;
    case 77:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW GRAINS";
      break;
    case 80:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN SHOWERS SLIGHT";
      break;
    case 81:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN SHOWERS MODERATE";
      break;
    case 82:
      lv_image_set_src(this->weather_image, &image_weather_rain);
      this->weather_description = "RAIN SHOWERS VIOLENT";
      break;
    case 85:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW SHOWERS SLIGHT";
      break;
    case 86:
      lv_image_set_src(this->weather_image, &image_weather_snow);
      this->weather_description = "SNOW SHOWERS HEAVY";
      break;
    case 95:
      lv_image_set_src(this->weather_image, &image_weather_thunder);
      this->weather_description = "THUNDERSTORM";
      break;
    case 96:
      lv_image_set_src(this->weather_image, &image_weather_thunder);
      this->weather_description = "THUNDERSTORM SLIGHT HAIL";
      break;
    case 99:
      lv_image_set_src(this->weather_image, &image_weather_thunder);
      this->weather_description = "THUNDERSTORM HEAVY HAIL";
      break;
    default: 
      this->weather_description = "UNKNOWN WEATHER CODE";
      break;
  }
}

// Store date and time
CurrentWeather::CurrentWeather() {
  text_label_apparent_temperature = nullptr;
  text_label_humidity = nullptr;
  text_label_wind_speed = nullptr;
  weather_image = nullptr;
  text_label_weather_description = nullptr;
};

CurrentWeather::CurrentWeather(const JsonDocument& doc):
  CurrentWeather()
{
  this->update_fields(doc, 0);
}

void CurrentWeather::update_fields(const JsonDocument& doc, int idx) {
  Serial.println("Update current weather fields from document: " + doc.as<String>());
  this->is_day = static_cast<int>(doc["current"]["is_day"]) == 1 ? true : false; // Convert to boolean
  Serial.println("Got current is_day status");
  this->apparent_temperature = static_cast<float>(doc["current"]["apparent_temperature"]);
  Serial.println("Got current apparent_temperature");
  this->humidity = static_cast<float>(doc["current"]["relative_humidity_2m"]);
  Serial.println("Got current humidity");
  this->wind_speed = static_cast<float>(doc["current"]["wind_speed_10m"]);
  Serial.println("Got wind speed");
  this->weather_code = static_cast<int>(doc["current"]["weather_code"]);
  Serial.println("Got current weather_code");
  this->print(); // Print the current weather data for debugging
}

void CurrentWeather::print() const {
  Serial.println("Current Weather:");
  Serial.print("Is Day: ");
  Serial.println(this->is_day);
  Serial.print("Apparent Temperature: ");
  Serial.println(this->apparent_temperature);
  Serial.print("Humidity: ");
  Serial.println(this->humidity);
  Serial.print("Wind speed: ");
  Serial.println(this->wind_speed);
  Serial.print("Weather Code: ");
  Serial.println(this->weather_code);
}

DailyWeather::DailyWeather() {
  text_label_apparent_min_temperature = nullptr;
  text_label_apparent_max_temperature = nullptr;
  text_label_sunrise = nullptr;
  text_label_sunset = nullptr;
  text_label_max_wind_speed = nullptr;
  text_label_max_precipitation_probability = nullptr;
  weather_image = nullptr;
  text_label_weather_description = nullptr;
};

DailyWeather::DailyWeather(const JsonDocument& doc, int idx):
  DailyWeather()
{
  this->update_fields(doc, idx);
}

void DailyWeather::update_fields(const JsonDocument& doc, int idx) {
  this->idx = idx; // Store the index for later use
  Serial.println("Update daily weather fields with index: " + String(this->idx) + " from document: " + doc.as<String>());
  Serial.println("Set weather code");
  this->weather_code = doc["daily"]["weather_code"][this->idx];
  Serial.println("Set temp max");
  this->max_apparent_temperature = doc["daily"]["apparent_temperature_max"][this->idx];
  Serial.println("Set temp min");
  this->min_apparent_temperature = doc["daily"]["apparent_temperature_min"][this->idx];
  Serial.println("Set sunrise");
  this->sunrise = String(doc["daily"]["sunrise"][this->idx].as<const char*>());
  Serial.println("Set sunset");
  this->sunset = String(doc["daily"]["sunset"][this->idx].as<const char*>());
  Serial.println("Set wind");
  this->max_wind_speed = doc["daily"]["wind_speed_10m_max"][this->idx];
  Serial.println("Set max precipitation probability");
  this->max_precipitation_probability = doc["daily"]["precipitation_probability_max"][this->idx];
  Serial.println("Set rainy");
  this->is_rainy = this->max_precipitation_probability > 50;
  this->print(); // Print the current weather data for debugging
}

void DailyWeather::print() const {
  Serial.println("Daily Weather:");
  Serial.print("Index: ");
  Serial.println(this->idx);
  Serial.print("Weather code: ");
  Serial.println(this->weather_code);
  Serial.print("Max apparent temperature: ");
  Serial.println(this->max_apparent_temperature);
  Serial.print("Min apparent temperature: ");
  Serial.println(this->min_apparent_temperature);
  Serial.print("Sunrise: ");
  Serial.println(this->sunrise);
  Serial.print("Sunset: ");
  Serial.println(this->sunset);
  Serial.print("Max wind speed: ");
  Serial.println(this->max_wind_speed);
  Serial.print("Max precipitation probability: ");
  Serial.println(this->max_precipitation_probability);
  Serial.print("Rainy: ");
  Serial.println(this->is_rainy);
}

JsonDocument get_weather_data() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Construct the API endpoint
    String url = String(
      base_url + "&current=wind_speed_10m,apparent_temperature,relative_humidity_2m,is_day,weather_code" +
      "&daily=apparent_temperature_max,apparent_temperature_min,weather_code,sunrise,sunset,precipitation_probability_max,wind_speed_10m_max"
    );
    Serial.print("Connecting to Open Meteo API");
    Serial.println(url);
    http.begin(url);
    Serial.println("Fetching weather data");
    int httpCode = http.GET(); // Make the GET request

    if (httpCode > 0) {
      // Check for the response
      if (httpCode == HTTP_CODE_OK) {
        Serial.println("Got response from Open Meteo API");
        String payload = http.getString();
        Serial.print("Got payload: ");
        Serial.println(payload);
        JsonDocument doc;
        Serial.println("Deserializing JSON");
        DeserializationError error = deserializeJson(doc, payload);
        if (!error) {
          Serial.println("No error in deserialization");
          return doc;
        } else {
          Serial.print("deserializeJson() failed: ");
          Serial.println(error.c_str());
        }
      }
      else {
        Serial.println("Failed");
      }
    } else {
      Serial.printf("GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end(); // Close connection
  } else {
    Serial.println("Not connected to Wi-Fi");
  }
  return JsonDocument{}; // Return empty object if not connected to Wi-Fi
}

String datetime_string_date(String datetime) {
  Serial.print("Extracting date from datetime: ");
  Serial.println(datetime);
  int splitIndex = datetime.indexOf('T');
  return datetime.substring(0, splitIndex); // Extract day portion
}

String datetime_string_time(String datetime) {
  Serial.print("Extracting time from datetime: ");
  Serial.println(datetime);
  int splitIndex = datetime.indexOf('T');
  return datetime.substring(splitIndex + 1, splitIndex + 9); // Extract time portion
}

String update_last_weather_update(const JsonDocument& doc) {
  const char* current_time = doc["current"]["time"];
  Serial.print("Current time from document: ");
  Serial.println(current_time);
  return String(current_time);
}