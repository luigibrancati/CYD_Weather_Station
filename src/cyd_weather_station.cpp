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

#include "weather_images.h"
#include "weather_data.h"
#include "sensor.h"
#include <lvgl.h>

/*  Install the "TFT_eSPI" library by Bodmer to interface with the TFT Display - https://github.com/Bodmer/TFT_eSPI
    *** IMPORTANT: User_Setup.h available on the internet will probably NOT work with the examples available at Random Nerd Tutorials ***
    *** YOU MUST USE THE User_Setup.h FILE PROVIDED IN THE LINK BELOW IN ORDER TO USE THE EXAMPLES FROM RANDOM NERD TUTORIALS ***
    FULL INSTRUCTIONS AVAILABLE ON HOW CONFIGURE THE LIBRARY: https://RandomNerdTutorials.com/cyd-lvgl/ or https://RandomNerdTutorials.com/esp32-tft-lvgl/   */
#include <TFT_eSPI.h>
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
#define WEATHER_IMAGE_POS_Y -30
// Weather description position
#define WEATHER_DESCRIPTION_POS_X -70
#define WEATHER_DESCRIPTION_POS_Y WEATHER_IMAGE_POS_Y + 60
#define WEATHER_DESCRIPTION_LEN 140
// Temperature image position
#define TEMPERATURE_IMAGE_POS_X -5
#define TEMPERATURE_IMAGE_POS_Y WEATHER_IMAGE_POS_Y - 30
// Humidity image position
#define HUMIDITY_IMAGE_POS_X -5
#define HUMIDITY_IMAGE_POS_Y WEATHER_IMAGE_POS_Y
// Wind image position
#define WIND_IMAGE_POS_X -5
#define WIND_IMAGE_POS_Y WEATHER_IMAGE_POS_Y + 30

#define ICONS_TEXT_PADDING 90

#define DRAW_BUF_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))

// SET VARIABLE TO 0 FOR TEMPERATURE IN FAHRENHEIT DEGREES
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

int x, y, z;
lv_obj_t * tabview;
lv_obj_t * tab_current;
lv_obj_t * tab_today;
lv_obj_t * tab_tomorrow;

CurrentWeather current_weather;
DailyWeather today_weather, tomorrow_weather;
SensorData sensor_data;

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

void update_data(lv_timer_t * timer) {
  Serial.println("Update data timer callback");
  Serial.println("Get the weather data from open-meteo.com API");
  LV_UNUSED(timer);
  // First get data from sensor
  sensor_data.read_sensor();
  sensor_data.print();
  // Then get the weather data
  Serial.println("Get the weather data");
  // Get the weather data from the API
  // The get_weather_data() function returns a JsonDocument with the weather data
  // If the JsonDocument is null, it means that the data was not retrieved successfully
  // If the JsonDocument is not null, it means that the data was retrieved successfully
  // The JsonDocument is then used to update the weather data on the screen
  JsonDocument doc = get_weather_data();
  if (doc.isNull()) {
    Serial.println("Failed to get weather data");
    return;
  } else {
    Serial.println("Got weather data");
    // Update the last weather update time
    Serial.println("Update last weather update");
    last_weather_update = update_last_weather_update(doc);
    Serial.print("Last weather update time: " + last_weather_update);
    String last_update_string = String("Last Update: " + datetime_string_time(last_weather_update) + "  |  " + location);
    Serial.println("Set last update label text: " + last_update_string);
    lv_label_set_text(text_label_time_location, last_update_string.c_str());
    Serial.print("Set current time label: ");
    Serial.print(String(text_label_time_location->coords.x1) + ' ');
    Serial.print(String(text_label_time_location->coords.x2) + ' ');
    Serial.print(String(text_label_time_location->coords.y1) + ' ');
    Serial.println(String(text_label_time_location->coords.y2));
    // Update current weather data
    Serial.println("Update current weather");
    current_weather.update_fields(doc, -1);
    current_weather.get_weather_description(current_weather.is_day, current_weather.weather_code);
    Serial.println("Set current weather image");
    lv_label_set_text(current_weather.text_label_apparent_temperature, (String(sensor_data.temperature) + degree_symbol).c_str());
    Serial.println("Set current weather temperature label");
    lv_label_set_text(current_weather.text_label_humidity, (String(sensor_data.humidity) + "%").c_str());
    Serial.println("Set current weather humidity label");
    lv_label_set_text(current_weather.text_label_wind_speed, (String(current_weather.wind_speed) + " km/h").c_str());
    Serial.println("Set current weather wind speed label");
    lv_label_set_text(current_weather.text_label_weather_description, current_weather.weather_description.c_str());
    Serial.println("Set current weather description label");
    // Update today weather data
    Serial.println("Update today weather");
    today_weather.update_fields(doc, 0);
    today_weather.get_weather_description(current_weather.is_day, today_weather.weather_code);
    Serial.println("Set today labels");
    lv_label_set_text(today_weather.text_label_apparent_min_temperature, (String(today_weather.min_apparent_temperature) + degree_symbol).c_str());
    lv_label_set_text(today_weather.text_label_apparent_max_temperature, (String(today_weather.max_apparent_temperature) + degree_symbol).c_str());
    lv_label_set_text(today_weather.text_label_sunrise, datetime_string_time(today_weather.sunrise).c_str());
    lv_label_set_text(today_weather.text_label_sunset, datetime_string_time(today_weather.sunset).c_str());
    lv_label_set_text(today_weather.text_label_max_wind_speed, (String(today_weather.max_wind_speed) + " km/h").c_str());
    lv_label_set_text(today_weather.text_label_max_precipitation_probability, (String(today_weather.max_precipitation_probability) + "%").c_str());
    lv_label_set_text(today_weather.text_label_weather_description, today_weather.weather_description.c_str());
    // Update tomorrow weather data
    Serial.println("Update tomorrow weather");
    tomorrow_weather.update_fields(doc, 1);
    tomorrow_weather.get_weather_description(current_weather.is_day, tomorrow_weather.weather_code);
    Serial.println("Set tomorrow labels");
    lv_label_set_text(tomorrow_weather.text_label_apparent_min_temperature, (String(tomorrow_weather.min_apparent_temperature)  + degree_symbol).c_str());
    lv_label_set_text(tomorrow_weather.text_label_apparent_max_temperature, (String(tomorrow_weather.max_apparent_temperature) + degree_symbol).c_str());
    lv_label_set_text(tomorrow_weather.text_label_sunrise, datetime_string_time(tomorrow_weather.sunrise).c_str());
    lv_label_set_text(tomorrow_weather.text_label_sunset, datetime_string_time(tomorrow_weather.sunset).c_str());
    lv_label_set_text(tomorrow_weather.text_label_max_wind_speed, (String(tomorrow_weather.max_wind_speed) + " km/h").c_str());
    lv_label_set_text(tomorrow_weather.text_label_max_precipitation_probability, (String(tomorrow_weather.max_precipitation_probability) + "%").c_str());
    lv_label_set_text(tomorrow_weather.text_label_weather_description, tomorrow_weather.weather_description.c_str());
  }
}

void create_image_label(lv_obj_t*& label, lv_obj_t* tab, int x, int y, int label_width = WEATHER_DESCRIPTION_LEN, const lv_font_t* font = &lv_font_montserrat_14) {
  label = lv_label_create(tab);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
  lv_obj_set_width(label, label_width);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_align(label, LV_ALIGN_CENTER, x, y);
  lv_obj_set_style_text_font(label, font, 0);
}

void current_weather_page(lv_obj_t*& tab) {
  Serial.println("Build the current weather page");
  Serial.println("Image and label for weather");
  current_weather.weather_image = lv_image_create(tab);
  lv_image_set_scale(current_weather.weather_image, 190);
  lv_obj_align(current_weather.weather_image, LV_ALIGN_CENTER, WEATHER_IMAGE_POS_X, WEATHER_IMAGE_POS_Y);

  Serial.println("Label for weather description");
  create_image_label(current_weather.text_label_weather_description, tab, WEATHER_DESCRIPTION_POS_X, WEATHER_DESCRIPTION_POS_Y + 10, WEATHER_DESCRIPTION_LEN, &lv_font_montserrat_16);

  Serial.println("Image and label for temperature");
  lv_obj_t * current_weather_image_temperature = lv_image_create(tab);
  lv_image_set_src(current_weather_image_temperature, &image_weather_temperature);
  lv_obj_align(current_weather_image_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(current_weather_image_temperature, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_image_recolor_opa(current_weather_image_temperature, LV_OPA_COVER, 0);
  create_image_label(current_weather.text_label_apparent_temperature, tab, TEMPERATURE_IMAGE_POS_X - 30 + ICONS_TEXT_PADDING, TEMPERATURE_IMAGE_POS_Y, 80, &lv_font_montserrat_16);

  Serial.println("Image and label for humidity");
  lv_obj_t * weather_image_humidity = lv_image_create(tab);
  lv_image_set_src(weather_image_humidity, &image_weather_humidity);
  lv_obj_align(weather_image_humidity, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_humidity, lv_palette_main(LV_PALETTE_CYAN), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_humidity, LV_OPA_COVER, 0);
  create_image_label(current_weather.text_label_humidity, tab, HUMIDITY_IMAGE_POS_X - 30 + ICONS_TEXT_PADDING, HUMIDITY_IMAGE_POS_Y, 80, &lv_font_montserrat_16);

  Serial.println("Image and label for wind speed");
  lv_obj_t * weather_image_wind = lv_image_create(tab);
  lv_image_set_src(weather_image_wind, &image_weather_wind);
  lv_obj_align(weather_image_wind, LV_ALIGN_CENTER, WIND_IMAGE_POS_X, WIND_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_wind, lv_palette_main(LV_PALETTE_TEAL), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_wind, LV_OPA_COVER, 0);
  create_image_label(current_weather.text_label_wind_speed, tab, WIND_IMAGE_POS_X - 15 + ICONS_TEXT_PADDING, WIND_IMAGE_POS_Y, 110, &lv_font_montserrat_16);
}

void day_weather_page(DailyWeather& weather, lv_obj_t*& tab){
  Serial.println("Build the day weather page");
  Serial.println("Image and label for weather");
  weather.weather_image = lv_image_create(tab);
  lv_image_set_scale(weather.weather_image, 128);
  lv_obj_align(weather.weather_image, LV_ALIGN_CENTER, WEATHER_IMAGE_POS_X - 20, WEATHER_IMAGE_POS_Y);

  Serial.println("Label for weather description");
  create_image_label(weather.text_label_weather_description, tab, WEATHER_DESCRIPTION_POS_X - 10, WEATHER_DESCRIPTION_POS_Y, WEATHER_DESCRIPTION_LEN, &lv_font_montserrat_12);

  Serial.println("Image and label for temperature");
  lv_obj_t * weather_image_low_temperature = lv_image_create(tab);
  lv_image_set_src(weather_image_low_temperature, &image_weather_temperature_low);
  lv_obj_align(weather_image_low_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X - 20, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_low_temperature, lv_palette_main(LV_PALETTE_BLUE), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_low_temperature, LV_OPA_COVER, 0);
  create_image_label(weather.text_label_apparent_min_temperature, tab, TEMPERATURE_IMAGE_POS_X - 50 + ICONS_TEXT_PADDING, TEMPERATURE_IMAGE_POS_Y, 80);

  lv_obj_t * weather_image_temperature = lv_image_create(tab);
  lv_image_set_src(weather_image_temperature, &image_weather_temperature);
  lv_obj_align(weather_image_temperature, LV_ALIGN_CENTER, TEMPERATURE_IMAGE_POS_X + 70, TEMPERATURE_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(weather_image_temperature, lv_palette_main(LV_PALETTE_RED), 0);
  lv_obj_set_style_image_recolor_opa(weather_image_temperature, LV_OPA_COVER, 0);
  create_image_label(weather.text_label_apparent_max_temperature, tab, TEMPERATURE_IMAGE_POS_X + 20 + ICONS_TEXT_PADDING, TEMPERATURE_IMAGE_POS_Y, 60);  

  Serial.println("Image and label for max wind speed");
  lv_obj_t * image_wind_speed = lv_image_create(tab);
  lv_image_set_src(image_wind_speed, &image_weather_wind);
  lv_obj_align(image_wind_speed, LV_ALIGN_CENTER, HUMIDITY_IMAGE_POS_X - 20, HUMIDITY_IMAGE_POS_Y);
  lv_obj_set_style_image_recolor(image_wind_speed, lv_palette_main(LV_PALETTE_TEAL), 0);
  lv_obj_set_style_image_recolor_opa(image_wind_speed, LV_OPA_COVER, 0);
  create_image_label(weather.text_label_max_wind_speed, tab, HUMIDITY_IMAGE_POS_X - 35 + ICONS_TEXT_PADDING, HUMIDITY_IMAGE_POS_Y, 110);

  Serial.println("Label for precipitation probability");
  lv_obj_t * image_precipitation_probability = lv_image_create(tab);
  lv_image_set_scale(image_precipitation_probability, 128);
  lv_image_set_src(image_precipitation_probability, &image_weather_precipitation_prob);
  lv_obj_align(image_precipitation_probability, LV_ALIGN_CENTER, WIND_IMAGE_POS_X - 20, WIND_IMAGE_POS_Y);
  create_image_label(weather.text_label_max_precipitation_probability, tab, WIND_IMAGE_POS_X - 50 + ICONS_TEXT_PADDING, WIND_IMAGE_POS_Y, 80);

  Serial.println("Label for sunrise and sunset");
  lv_obj_t * image_sunrise = lv_image_create(tab);
  lv_image_set_scale(image_sunrise, 200);
  lv_image_set_src(image_sunrise, &image_weather_sunrise);
  lv_obj_align(image_sunrise, LV_ALIGN_CENTER, WIND_IMAGE_POS_X - 20, WIND_IMAGE_POS_Y + 30);
  create_image_label(weather.text_label_sunrise, tab, WIND_IMAGE_POS_X - 45 + ICONS_TEXT_PADDING, WIND_IMAGE_POS_Y + 30, 80);

  lv_obj_t * image_sunset = lv_image_create(tab);
  lv_image_set_scale(image_sunset, 200);
  lv_image_set_src(image_sunset, &image_weather_sunset);
  lv_obj_align(image_sunset, LV_ALIGN_CENTER, WIND_IMAGE_POS_X + 70, WIND_IMAGE_POS_Y + 30);
  create_image_label(weather.text_label_sunset, tab, WIND_IMAGE_POS_X + 30 + ICONS_TEXT_PADDING, WIND_IMAGE_POS_Y + 30, 50);
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
  LV_IMAGE_DECLARE(image_weather_temperature_low);
  LV_IMAGE_DECLARE(image_weather_humidity);

  Serial.println("Create the main screen");
  lv_obj_t * scr = lv_screen_active();

  /*Create a Tab view object*/
  tabview = lv_tabview_create(scr);
  tab_current = lv_tabview_add_tab(tabview, "Current");
  tab_today = lv_tabview_add_tab(tabview, "Today");
  tab_tomorrow = lv_tabview_add_tab(tabview, "Tomorrow");

  current_weather_page(tab_current);
  day_weather_page(today_weather, tab_today);
  day_weather_page(tomorrow_weather, tab_tomorrow);
  // Create a text label for the time and timezone aligned center in the bottom of the screen
  Serial.println("Label for time and location");
  text_label_time_location = lv_label_create(scr);
  lv_obj_align(text_label_time_location, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_obj_set_style_text_font((lv_obj_t*) text_label_time_location, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color((lv_obj_t*) text_label_time_location, lv_palette_main(LV_PALETTE_GREY), 0);
  
  update_data(NULL);
  lv_timer_t * timer = lv_timer_create(update_data, 60000, NULL);
  lv_timer_ready(timer);
}

void connect_to_wifi() {
  unsigned long wait_time = 60000;
  unsigned long start = millis();
  lv_obj_t* wifi_label = lv_label_create(lv_screen_active());
  lv_obj_set_width(wifi_label, 300);
  lv_label_set_long_mode(wifi_label, LV_LABEL_LONG_WRAP);
  lv_obj_align(wifi_label, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(wifi_label, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(wifi_label, lv_palette_main(LV_PALETTE_GREY), 0);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to wifi");
  String wifi_text = String("Connecting to Wi-Fi");
  lv_label_set_text(wifi_label, wifi_text.c_str());
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(0);     // tell LVGL how much time has passed
  delay(500);
  while (WiFi.status() != WL_CONNECTED && (millis() - start < wait_time)) {
    Serial.print(".");
    wifi_text += ".";
    lv_label_set_text(wifi_label, wifi_text.c_str());
    lv_task_handler();  // let the GUI do its work
    lv_tick_inc(500);     // tell LVGL how much time has passed
    delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected to Wi-Fi network with IP Address: ");
    Serial.println(WiFi.localIP());
    lv_obj_clean(lv_screen_active());  // Clear the screen to remove the Wi-Fi label
    lv_task_handler();  // let the GUI do its work
    lv_tick_inc(0);     // tell LVGL how much time has passed
    // Function to draw the GUI
    Serial.println("Draw the GUI");
    lv_create_main_gui();
    return;
  } else {
    Serial.println("Failed to connect to Wi-Fi");
    lv_label_set_text(wifi_label, "Failed to connect to Wi-Fi");
    return;
  }
}

void setup() {
  String LVGL_Arduino = String("LVGL Library Version: ") + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
  Serial.begin(115200);
  Serial.println(LVGL_Arduino);
  sensor_data.init_sensor();
  Serial.println("Sensor initialized");
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
  // Connect to wifi
  connect_to_wifi();
}

void loop() {
  lv_task_handler();  // let the GUI do its work
  lv_tick_inc(5);     // tell LVGL how much time has passed
  delay(5);           // let this time pass
}