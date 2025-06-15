/*  Rui Santos & Sara Santos - Random Nerd Tutorials - https://RandomNerdTutorials.com/esp32-cyd-lvgl-weather-station/   |    https://RandomNerdTutorials.com/esp32-tft-lvgl-weather-station/
    THIS EXAMPLE WAS TESTED WITH THE FOLLOWING HARDWARE:
    1) ESP32-2432S028R 2.8 inch 240×320 also known as the Cheap Yellow Display (CYD): https://makeradvisor.com/tools/cyd-cheap-yellow-display-esp32-2432s028r/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/cyd-lvgl/
    2) REGULAR ESP32 Dev Board + 2.8 inch 240x320 TFT Display: https://makeradvisor.com/tools/2-8-inch-ili9341-tft-240x320/ and https://makeradvisor.com/tools/esp32-dev-board-wi-fi-bluetooth/
      SET UP INSTRUCTIONS: https://RandomNerdTutorials.com/esp32-tft-lvgl/
    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/

/*  Install the "lvgl" library version 9.X by kisvegabor to interface with the TFT Display - https://lvgl.io/
    *** IMPORTANT: lv_conf.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE lv_conf.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>

#include "weather_images.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <XPT2046_Touchscreen.h>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320
// Touchscreen pins
#define XPT2046_IRQ 36   // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK 25   // T_CLK
#define XPT2046_CS 33    // T_CS
// Weather image position
#define WEATHER_IMAGE_POS_X -90
#define WEATHER_IMAGE_POS_Y -10
// Weather description position
#define WEATHER_DESCRIPTION_POS_X 65
#define WEATHER_DESCRIPTION_POS_Y 20
// Temperature image position
#define TEMPERATURE_IMAGE_POS_X 0
#define TEMPERATURE_IMAGE_POS_Y -50
// Humidity image position
#define HUMIDITY_IMAGE_POS_X 0
#define HUMIDITY_IMAGE_POS_Y -20

#define ICONS_TEXT_PADDING 50

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

// SET VARIABLE TO 0 FOR TEMPERATURE IN FAHRENHEIT DEGREES
static const char degree_symbol[] = "\u00B0C";
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

const char* ssid = "FASTWEB-FC83A5";
const char* password = "MERHGZ6ECG";
String latitude = "45.07049768682";
String longitude = "7.68682";
String location = "Turin";
String timezone = "Europe/Rome";

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int x, y, z;
lv_obj_t * tabview;
lv_obj_t * tab_current;
lv_obj_t * tab_forecast;

// If logging is enabled, it will inform the user about what is happening in the library
void log_print(lv_log_level_t level, const char * buf) {
  LV_UNUSED(level);
  Serial.println(buf);
  Serial.flush();
}

// Get the Touchscreen data
void touchscreen_read(lv_indev_t * indev, lv_indev_data_t * data) {
  // Checks if Touchscreen was touched, and prints X, Y and Pressure (Z)
  if(touchscreen.tirqTouched() && touchscreen.touched()) {
    // Get Touchscreen points
    TS_Point p = touchscreen.getPoint();
    // Calibrate Touchscreen points with map function to the correct width and height
    x = map(p.x, 200, 3700, 1, SCREEN_WIDTH);
    y = map(p.y, 240, 3800, 1, SCREEN_HEIGHT);
    z = p.z;

    data->state = LV_INDEV_STATE_PRESSED;

    // Set the coordinates
    data->point.x = x;
    data->point.y = y;

    // Print Touchscreen info about X, Y and Pressure (Z) on the Serial Monitor
    Serial.print("X = ");
    Serial.print(x);
    Serial.print(" | Y = ");
    Serial.print(y);
    Serial.print(" | Pressure = ");
    Serial.print(z);
    Serial.println();
  }
  else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

struct Weather {
  String weather_description;
  lv_obj_t * weather_image;
  lv_obj_t * text_label_weather_description;

  Weather(){
    weather_image = nullptr;
    text_label_weather_description = nullptr;
  };

  virtual void update_fields(const JsonDocument& doc) = 0;

  /*
    WMO Weather interpretation codes (WW)- Code	Description
    0	Clear sky
    1, 2, 3	Mainly clear, partly cloudy, and overcast
    45, 48	Fog and depositing rime fog
    51, 53, 55	Drizzle: Light, moderate, and dense intensity
    56, 57	Freezing Drizzle: Light and dense intensity
    61, 63, 65	Rain: Slight, moderate and heavy intensity
    66, 67	Freezing Rain: Light and heavy intensity
    71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
    77	Snow grains
    80, 81, 82	Rain showers: Slight, moderate, and violent
    85, 86	Snow showers slight and heavy
    95 *	Thunderstorm: Slight or moderate
    96, 99 *	Thunderstorm with slight and heavy hail
  */
  void get_weather_description(bool is_day, int weather_code) {
    switch (weather_code) {
      case 0:
        if(is_day==1) {
          lv_image_set_src(this->weather_image, &image_weather_sun); }
        else { lv_image_set_src(this->weather_image, &image_weather_night); }
        this->weather_description = "CLEAR SKY";
        break;
      case 1: 
        if(is_day==1) { lv_image_set_src(this->weather_image, &image_weather_sun); }
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

  CurrentWeather() {
    text_label_date = nullptr;
    text_label_temperature = nullptr;
    text_label_humidity = nullptr;
    text_label_time_location = nullptr;
    weather_image = nullptr;
    text_label_weather_description = nullptr;
  };

  CurrentWeather(const JsonDocument& doc):
    CurrentWeather()
  {
    this->update_fields(doc);
  }

  void update_fields(const JsonDocument& doc) override {
    const char* current_time = doc["current"]["time"];
    this->last_weather_update = String(current_time);
    Serial.println("Got last weather update time");
    Serial.print("Is day print");
    Serial.println(doc["current"]["is_day"].as<int>());
    this->is_day = doc["current"]["is_day"].as<int>() == 1 ? true : false; // Convert to boolean
    Serial.println("Got current is_day status");
    this->temperature = static_cast<float>(doc["current"]["temperature_2m"]);
    Serial.println("Got current temperature");
    this->apparent_temperature = static_cast<float>(doc["current"]["apparent_temperature"]);
    Serial.println("Got current apparent_temperature");
    this->humidity = static_cast<float>(doc["current"]["relative_humidity_2m"]);
    Serial.println("Got current humidity");
    this->weather_code = static_cast<int>(doc["current"]["weather_code"]);
    Serial.println("Got current weather_code");
  }

  const String last_update_day() const {
    int splitIndex = this->last_weather_update.indexOf('T');
    return this->last_weather_update.substring(0, splitIndex); // Extract day portion
  }

  const String last_update_time() const {
    int splitIndex = this->last_weather_update.indexOf('T');
    return this->last_weather_update.substring(splitIndex + 1, splitIndex + 9); // Extract time portion
  }
} current_weather;

// Store date and time
struct ForecastWeather : Weather {
  bool is_rainy;
  int max_weather_code;
  float max_apparent_temperature;
  float min_apparent_temperature;
  lv_obj_t * text_label_max_temperature;
  lv_obj_t * text_label_min_temperature;

  ForecastWeather() {
    weather_image = nullptr;
    text_label_weather_description = nullptr;
    text_label_max_temperature = nullptr;
    text_label_min_temperature = nullptr;
  };

  ForecastWeather(const JsonDocument& doc):
    ForecastWeather()
  {
    this->update_fields(doc);
  }

  void update_fields(const JsonDocument& doc) {
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
  }
  
} forecast_weather;

JsonDocument get_weather_data() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // Construct the API endpoint
    String url = String(
      "http://api.open-meteo.com/v1/forecast?latitude=" + latitude + "&longitude=" + longitude + "&current=relative_humidity_2m,temperature_2m,weather_code,apparent_temperature&hourly=relative_humidity_2m,precipitation_probability,temperature_2m,apparent_temperature,weather_code&timezone=" + timezone + "&forecast_days=3"
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
        Serial.println("Getting payload as string");
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

static void update_data(lv_timer_t * timer){
  Serial.println("Update data timer callback");
  Serial.println("Get the weather data from open-meteo.com API");
  LV_UNUSED(timer);
  JsonDocument doc = get_weather_data();
  if (doc.isNull()) {
    Serial.println("Failed to get weather data");
    return;
  } else {
    Serial.println("Got weather data");
    // Update current weather data
    Serial.println("Update current");
    current_weather.update_fields(doc);
    current_weather.get_weather_description(current_weather.is_day, current_weather.weather_code);
    lv_label_set_text(current_weather.text_label_date, current_weather.last_update_day().c_str());
    lv_label_set_text(current_weather.text_label_temperature, String(String(current_weather.temperature) + degree_symbol).c_str());
    lv_label_set_text(current_weather.text_label_humidity, String(String(current_weather.humidity) + "%").c_str());
    lv_label_set_text(current_weather.text_label_weather_description, current_weather.weather_description.c_str());
    lv_label_set_text(current_weather.text_label_time_location, String("Last Update: " + current_weather.last_update_time() + "  |  " + location).c_str());
    // Update the forecast weather data
    Serial.println("Update forecast");
    forecast_weather.update_fields(doc);
    forecast_weather.get_weather_description(current_weather.is_day, forecast_weather.max_weather_code);
    lv_label_set_text(forecast_weather.text_label_max_temperature, String(String(forecast_weather.max_apparent_temperature) + degree_symbol).c_str());
    lv_label_set_text(forecast_weather.text_label_min_temperature, String(String(forecast_weather.min_apparent_temperature) + degree_symbol).c_str());
    lv_label_set_text(forecast_weather.text_label_weather_description, forecast_weather.weather_description.c_str());
  }
}

void lv_create_main_gui(void) {
  Serial.println("Declare images");
  LV_IMAGE_DECLARE(image_weather_sun);
  LV_IMAGE_DECLARE(image_weather_cloud);
  LV_IMAGE_DECLARE(image_weather_rain);
  LV_IMAGE_DECLARE(image_weather_thunder);
  LV_IMAGE_DECLARE(image_weather_snow);
  LV_IMAGE_DECLARE(image_weather_night);
  LV_IMAGE_DECLARE(image_weather_temperature);
  LV_IMAGE_DECLARE(image_weather_humidity);

  Serial.println("Create the main screen");
  lv_obj_t * scr = lv_screen_active();

  /*Create a Tab view object*/
  tabview = lv_tabview_create(scr);

  /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
  tab_current = lv_tabview_add_tab(tabview, "Current");
  tab_forecast = lv_tabview_add_tab(tabview, "Forecast");

  Serial.println("Labels for date");
  current_weather.text_label_date = lv_label_create(tab_current);
  lv_obj_align(current_weather.text_label_date, LV_ALIGN_CENTER, 0, -100);
  lv_obj_set_style_text_font((lv_obj_t*) current_weather.text_label_date, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_color((lv_obj_t*) current_weather.text_label_date, lv_palette_main(LV_PALETTE_CYAN), 0);

  Serial.println("Image and label for current weather");
  current_weather.weather_image = lv_image_create(tab_current);
  lv_image_set_scale(current_weather.weather_image, 228);
  lv_obj_align(current_weather.weather_image, LV_ALIGN_CENTER, WEATHER_IMAGE_POS_X, WEATHER_IMAGE_POS_Y);

  forecast_weather.weather_image = lv_image_create(tab_forecast);
  lv_image_set_scale(current_weather.weather_image, 228);
  lv_obj_align(forecast_weather.weather_image, LV_ALIGN_CENTER, WEATHER_IMAGE_POS_X, WEATHER_IMAGE_POS_Y);

  Serial.println("Image and label for temperature");
  lv_obj_t * weather_image_temperature = lv_image_create(tab_current);
  lv_image_set_src(weather_image_temperature, &image_weather_temperature);
  lv_obj_align(weather_image_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_temperature, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_temperature, LV_OPA_COVER, 0);
  current_weather.text_label_temperature = lv_label_create(tab_current);
  lv_obj_align(current_weather.text_label_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X + ICONS_TEXT_PADDING, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) current_weather.text_label_temperature, &lv_font_montserrat_18, 0);

  lv_obj_t * weather_image_min_temperature = lv_image_create(tab_forecast);
  lv_image_set_src(weather_image_min_temperature, &image_weather_temperature);
  lv_obj_align(weather_image_min_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_min_temperature, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_min_temperature, LV_OPA_COVER, 0);
  forecast_weather.text_label_min_temperature = lv_label_create(tab_forecast);
  lv_obj_align(forecast_weather.text_label_min_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X + ICONS_TEXT_PADDING, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) forecast_weather.text_label_min_temperature, &lv_font_montserrat_18, 0);

  lv_obj_t * weather_image_max_temperature = lv_image_create(tab_forecast);
  lv_image_set_src(weather_image_max_temperature, &image_weather_temperature);
  lv_obj_align(weather_image_max_temperature, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_max_temperature, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_max_temperature, LV_OPA_COVER, 0);
  forecast_weather.text_label_max_temperature = lv_label_create(tab_forecast);
  lv_obj_align(forecast_weather.text_label_max_temperature, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X + ICONS_TEXT_PADDING, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) forecast_weather.text_label_max_temperature, &lv_font_montserrat_18, 0);

  Serial.println("Image and label for humidity");
  lv_obj_t * weather_image_humidity = lv_image_create(tab_current);
  lv_image_set_src(weather_image_humidity, &image_weather_humidity);
  lv_obj_align(weather_image_humidity, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_humidity, lv_palette_main(LV_PALETTE_CYAN), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_humidity, LV_OPA_COVER, 0);
  current_weather.text_label_humidity = lv_label_create(tab_current);
  lv_obj_align(current_weather.text_label_humidity, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X + ICONS_TEXT_PADDING, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) current_weather.text_label_humidity, &lv_font_montserrat_18, 0);

  Serial.println("Label for weather description");
  current_weather.text_label_weather_description = lv_label_create(tab_current);
  lv_obj_set_style_text_align(current_weather.text_label_weather_description, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_width(current_weather.text_label_weather_description, 150);
  lv_label_set_long_mode(current_weather.text_label_weather_description, LV_LABEL_LONG_WRAP);
  lv_obj_align(current_weather.text_label_weather_description, LV_ALIGN_CENTER, WEATHER_DESCRIPTION_POS_X, WEATHER_DESCRIPTION_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) current_weather.text_label_weather_description, &lv_font_montserrat_16, 0);

  forecast_weather.text_label_weather_description = lv_label_create(tab_forecast);
  lv_obj_set_style_text_align(forecast_weather.text_label_weather_description, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_width(forecast_weather.text_label_weather_description, 150);
  lv_label_set_long_mode(forecast_weather.text_label_weather_description, LV_LABEL_LONG_WRAP);
  lv_obj_align(forecast_weather.text_label_weather_description, LV_ALIGN_CENTER, WEATHER_DESCRIPTION_POS_X, WEATHER_DESCRIPTION_POS_Y);
  lv_obj_set_style_text_font((lv_obj_t*) forecast_weather.text_label_weather_description, &lv_font_montserrat_16, 0);

  // Create a text label for the time and timezone aligned center in the bottom of the screen
  Serial.println("Label for time and location");
  current_weather.text_label_time_location = lv_label_create(scr);
  lv_obj_align(current_weather.text_label_time_location, LV_ALIGN_BOTTOM_MID, 0, -5);
  lv_obj_set_style_text_font((lv_obj_t*) current_weather.text_label_time_location, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color((lv_obj_t*) current_weather.text_label_time_location, lv_palette_main(LV_PALETTE_GREY), 0);
  
  update_data(NULL);
  lv_timer_t * timer = lv_timer_create(update_data, 60000, NULL);
  lv_timer_ready(timer);
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nConnected to Wi-Fi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start LVGL
  Serial.println("Init LVGL");
  lv_init();
  // Register print function for debugging
  Serial.println("Register print function for debugging");
  lv_log_register_print_cb(log_print);

  // Start the SPI for the touchscreen and init the touchscreen
  Serial.println("Start the SPI for the touchscreen and init the touchscreen");
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Set the Touchscreen rotation in landscape mode
  // Note: in some displays, the touchscreen might be upside down, so you might need to set the rotation to 0: touchscreen.setRotation(0);
  touchscreen.setRotation(2);

  // Create a display object
  Serial.println("Create a display object");
  lv_display_t * disp;
  // Initialize the TFT display using the TFT_eSPI library
  Serial.println("Initialize the TFT display using the TFT_eSPI library");
  disp = lv_tft_espi_create(SCREEN_WIDTH, SCREEN_HEIGHT, draw_buf, sizeof(draw_buf));
  Serial.println("Rotate the display to 270 degrees");
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_270);
  // Initialize an LVGL input device object (Touchscreen)
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  // Set the callback function to read Touchscreen input
  lv_indev_set_read_cb(indev, touchscreen_read);
  // Function to draw the GUI
  Serial.println("Draw the GUI");
  lv_create_main_gui();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}