#include "weather_data.h"
#include "weather_images.h"
#include <WiFi.h>
#include <HTTPClient.h>

Weather::Weather(){
  weather_image = nullptr;
  text_label_weather_description = nullptr;
};

void Weather::get_weather_description(bool is_day, int weather_code) {
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
      lv_image_set_src(this->weather_image, &image_weather_cloud);
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
  text_label_date = nullptr;
  text_label_temperature = nullptr;
  text_label_humidity = nullptr;
  text_label_time_location = nullptr;
  weather_image = nullptr;
  text_label_weather_description = nullptr;
};

CurrentWeather::CurrentWeather(const JsonDocument& doc):
  CurrentWeather()
{
  this->update_fields(doc);
}

void CurrentWeather::update_fields(const JsonDocument& doc) {
  const char* current_time = doc["current"]["time"];
  this->last_weather_update = String(current_time);
  Serial.println("Got last weather update time");
  this->is_day = static_cast<int>(doc["current"]["is_day"]) == 1 ? true : false; // Convert to boolean
  Serial.println("Got current is_day status");
  this->temperature = static_cast<float>(doc["current"]["temperature_2m"]);
  Serial.println("Got current temperature");
  this->apparent_temperature = static_cast<float>(doc["current"]["apparent_temperature"]);
  Serial.println("Got current apparent_temperature");
  this->humidity = static_cast<float>(doc["current"]["relative_humidity_2m"]);
  Serial.println("Got current humidity");
  this->weather_code = static_cast<int>(doc["current"]["weather_code"]);
  Serial.println("Got current weather_code");
  this->print(); // Print the current weather data for debugging
}

const String CurrentWeather::last_update_day() const {
  int splitIndex = this->last_weather_update.indexOf('T');
  return this->last_weather_update.substring(0, splitIndex); // Extract day portion
}

const String CurrentWeather::last_update_time() const {
  int splitIndex = this->last_weather_update.indexOf('T');
  return this->last_weather_update.substring(splitIndex + 1, splitIndex + 9); // Extract time portion
}

void CurrentWeather::print() const {
  Serial.println("Current Weather:");
  Serial.print("Last Update: ");
  Serial.println(this->last_weather_update);
  Serial.print("Is Day: ");
  Serial.println(this->is_day);
  Serial.print("Temperature: ");
  Serial.print(this->temperature);
  Serial.print("Apparent Temperature: ");
  Serial.print(this->apparent_temperature);
  Serial.print("Humidity: ");
  Serial.print(this->humidity);
  Serial.print("Weather Code: ");
  Serial.println(this->weather_code);
}

ForecastWeather::ForecastWeather() {
  weather_image = nullptr;
  text_label_weather_description = nullptr;
  text_label_max_temperature = nullptr;
  text_label_min_temperature = nullptr;
};

ForecastWeather::ForecastWeather(const JsonDocument& doc):
  ForecastWeather()
{
  this->update_fields(doc);
}

void ForecastWeather::update_fields(const JsonDocument& doc) {
  Serial.println("Update forecast weather fields");
  this->max_weather_code = doc["hourly"]["weather_code"][0];
  this->max_apparent_temperature = doc["hourly"]["apparent_temperature"][0];
  this->min_apparent_temperature = doc["hourly"]["apparent_temperature"][0];
  this->is_rainy = false; // Default to no rain
  for (unsigned short i = 0; i < 24; i++) {
    if (doc["hourly"]["apparent_temperature"][i] > this->max_apparent_temperature) {
      this->max_apparent_temperature = doc["hourly"]["apparent_temperature"][i];
    } else if (doc["hourly"]["apparent_temperature"][i] < this->min_apparent_temperature) {
      this->min_apparent_temperature = doc["hourly"]["apparent_temperature"][i];
    }
    if (doc["hourly"]["precipitation_probability"][i] > 50) {
      this->is_rainy = true; // If any hour has a precipitation probability greater than 50%, it will rain
    }
    if (doc["hourly"]["weather_code"][i] > this->max_weather_code) {
      this->max_weather_code = doc["hourly"]["weather_code"][i];
    }
  }
  this->print(); // Print the current weather data for debugging
}

void ForecastWeather::print() const {
  Serial.println("Forecast Weather:");
  Serial.print("Rainy: ");
  Serial.println(this->is_rainy);
  Serial.print("Max weather code: ");
  Serial.println(this->max_weather_code);
  Serial.print("Max temperature: ");
  Serial.print(this->max_apparent_temperature);
  Serial.print("Min temperature: ");
  Serial.print(this->min_apparent_temperature);
}

JsonDocument get_weather_data() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Construct the API endpoint
    String url = String(
      "https://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=relative_humidity_2m,temperature_2m,weather_code,apparent_temperature,is_day&hourly=precipitation_probability,apparent_temperature,weather_code&timezone=" + timezone + "&forecast_days=1"
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