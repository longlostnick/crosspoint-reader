#include "WeatherProvider.h"

const char* WeatherProvider::getConditionDescription(WeatherCondition condition) {
  switch (condition) {
    case WeatherCondition::CLEAR:
      return "Clear";
    case WeatherCondition::MAINLY_CLEAR:
      return "Mainly Clear";
    case WeatherCondition::PARTLY_CLOUDY:
      return "Partly Cloudy";
    case WeatherCondition::OVERCAST:
      return "Overcast";
    case WeatherCondition::FOG:
      return "Fog";
    case WeatherCondition::DRIZZLE_LIGHT:
    case WeatherCondition::DRIZZLE:
    case WeatherCondition::DRIZZLE_HEAVY:
      return "Drizzle";
    case WeatherCondition::FREEZING_DRIZZLE:
      return "Freezing Drizzle";
    case WeatherCondition::RAIN_LIGHT:
      return "Light Rain";
    case WeatherCondition::RAIN:
      return "Rain";
    case WeatherCondition::RAIN_HEAVY:
      return "Heavy Rain";
    case WeatherCondition::FREEZING_RAIN:
      return "Freezing Rain";
    case WeatherCondition::SNOW_LIGHT:
      return "Light Snow";
    case WeatherCondition::SNOW:
      return "Snow";
    case WeatherCondition::SNOW_HEAVY:
      return "Heavy Snow";
    case WeatherCondition::SNOW_GRAINS:
      return "Snow Grains";
    case WeatherCondition::SHOWERS_LIGHT:
      return "Light Showers";
    case WeatherCondition::SHOWERS:
      return "Showers";
    case WeatherCondition::SHOWERS_HEAVY:
      return "Heavy Showers";
    case WeatherCondition::SNOW_SHOWERS_LIGHT:
    case WeatherCondition::SNOW_SHOWERS_HEAVY:
      return "Snow Showers";
    case WeatherCondition::THUNDERSTORM:
    case WeatherCondition::THUNDERSTORM_HAIL_LIGHT:
    case WeatherCondition::THUNDERSTORM_HAIL_HEAVY:
      return "Thunderstorm";
    default:
      return "Unknown";
  }
}

const char* WeatherProvider::getConditionIcon(WeatherCondition condition) {
  switch (condition) {
    case WeatherCondition::CLEAR:
    case WeatherCondition::MAINLY_CLEAR:
      return "O";  // Sun icon
    case WeatherCondition::PARTLY_CLOUDY:
      return "o~";  // Sun with cloud
    case WeatherCondition::OVERCAST:
      return "~~";  // Clouds
    case WeatherCondition::FOG:
      return "===";  // Fog lines
    case WeatherCondition::DRIZZLE_LIGHT:
    case WeatherCondition::DRIZZLE:
    case WeatherCondition::DRIZZLE_HEAVY:
    case WeatherCondition::RAIN_LIGHT:
    case WeatherCondition::RAIN:
    case WeatherCondition::RAIN_HEAVY:
    case WeatherCondition::FREEZING_RAIN:
    case WeatherCondition::FREEZING_DRIZZLE:
    case WeatherCondition::SHOWERS_LIGHT:
    case WeatherCondition::SHOWERS:
    case WeatherCondition::SHOWERS_HEAVY:
      return "///";  // Rain
    case WeatherCondition::SNOW_LIGHT:
    case WeatherCondition::SNOW:
    case WeatherCondition::SNOW_HEAVY:
    case WeatherCondition::SNOW_GRAINS:
    case WeatherCondition::SNOW_SHOWERS_LIGHT:
    case WeatherCondition::SNOW_SHOWERS_HEAVY:
      return "***";  // Snow
    case WeatherCondition::THUNDERSTORM:
    case WeatherCondition::THUNDERSTORM_HAIL_LIGHT:
    case WeatherCondition::THUNDERSTORM_HAIL_HEAVY:
      return "/!\\";  // Lightning
    default:
      return "?";
  }
}
