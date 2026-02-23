#pragma once

#include <cstdint>
#include <string>

/**
 * Weather condition codes (based on WMO Weather interpretation codes)
 */
enum class WeatherCondition : uint8_t {
  CLEAR = 0,
  MAINLY_CLEAR = 1,
  PARTLY_CLOUDY = 2,
  OVERCAST = 3,
  FOG = 45,
  DRIZZLE_LIGHT = 51,
  DRIZZLE = 53,
  DRIZZLE_HEAVY = 55,
  FREEZING_DRIZZLE = 56,
  RAIN_LIGHT = 61,
  RAIN = 63,
  RAIN_HEAVY = 65,
  FREEZING_RAIN = 66,
  SNOW_LIGHT = 71,
  SNOW = 73,
  SNOW_HEAVY = 75,
  SNOW_GRAINS = 77,
  SHOWERS_LIGHT = 80,
  SHOWERS = 81,
  SHOWERS_HEAVY = 82,
  SNOW_SHOWERS_LIGHT = 85,
  SNOW_SHOWERS_HEAVY = 86,
  THUNDERSTORM = 95,
  THUNDERSTORM_HAIL_LIGHT = 96,
  THUNDERSTORM_HAIL_HEAVY = 99,
  UNKNOWN = 255
};

/**
 * Weather data structure for a single day
 */
struct WeatherData {
  bool valid = false;
  float temperatureHigh;   // High temperature in Celsius
  float temperatureLow;    // Low temperature in Celsius
  float temperatureCurrent;  // Current temperature in Celsius
  int precipitationChance;   // Chance of precipitation (0-100%)
  float precipitationSum;    // Total precipitation in mm
  WeatherCondition condition;
  std::string locationName;
  
  // Convert Celsius to Fahrenheit
  static float toFahrenheit(float celsius) {
    return celsius * 9.0f / 5.0f + 32.0f;
  }
};

/**
 * Abstract interface for weather data providers.
 * Allows swapping weather API backends without changing application code.
 */
class WeatherProvider {
public:
  virtual ~WeatherProvider() = default;
  
  /**
   * Fetch current weather data for the given coordinates.
   * @param latitude Location latitude
   * @param longitude Location longitude
   * @return WeatherData with valid=true on success, valid=false on failure
   */
  virtual WeatherData fetchWeather(float latitude, float longitude) = 0;
  
  /**
   * Get a human-readable description of the weather condition
   */
  static const char* getConditionDescription(WeatherCondition condition);
  
  /**
   * Get an icon character for the weather condition (for e-ink display)
   */
  static const char* getConditionIcon(WeatherCondition condition);
};
