#include "OpenMeteoProvider.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include <cstdio>

#include "network/HttpDownloader.h"

static WeatherCondition parseWeatherCode(int code) {
  switch (code) {
    case 0:
      return WeatherCondition::CLEAR;
    case 1:
      return WeatherCondition::MAINLY_CLEAR;
    case 2:
      return WeatherCondition::PARTLY_CLOUDY;
    case 3:
      return WeatherCondition::OVERCAST;
    case 45:
    case 48:
      return WeatherCondition::FOG;
    case 51:
      return WeatherCondition::DRIZZLE_LIGHT;
    case 53:
      return WeatherCondition::DRIZZLE;
    case 55:
      return WeatherCondition::DRIZZLE_HEAVY;
    case 56:
    case 57:
      return WeatherCondition::FREEZING_DRIZZLE;
    case 61:
      return WeatherCondition::RAIN_LIGHT;
    case 63:
      return WeatherCondition::RAIN;
    case 65:
      return WeatherCondition::RAIN_HEAVY;
    case 66:
    case 67:
      return WeatherCondition::FREEZING_RAIN;
    case 71:
      return WeatherCondition::SNOW_LIGHT;
    case 73:
      return WeatherCondition::SNOW;
    case 75:
      return WeatherCondition::SNOW_HEAVY;
    case 77:
      return WeatherCondition::SNOW_GRAINS;
    case 80:
      return WeatherCondition::SHOWERS_LIGHT;
    case 81:
      return WeatherCondition::SHOWERS;
    case 82:
      return WeatherCondition::SHOWERS_HEAVY;
    case 85:
      return WeatherCondition::SNOW_SHOWERS_LIGHT;
    case 86:
      return WeatherCondition::SNOW_SHOWERS_HEAVY;
    case 95:
      return WeatherCondition::THUNDERSTORM;
    case 96:
      return WeatherCondition::THUNDERSTORM_HAIL_LIGHT;
    case 99:
      return WeatherCondition::THUNDERSTORM_HAIL_HEAVY;
    default:
      return WeatherCondition::UNKNOWN;
  }
}

WeatherData OpenMeteoProvider::fetchWeather(float latitude, float longitude) {
  WeatherData result;
  result.valid = false;

  char url[256];
  snprintf(url, sizeof(url),
           "%s?latitude=%.4f&longitude=%.4f"
           "&current=temperature_2m,weather_code"
           "&daily=temperature_2m_max,temperature_2m_min,precipitation_probability_max,precipitation_sum,weather_code"
           "&timezone=auto&forecast_days=1",
           API_BASE_URL, latitude, longitude);

  LOG_DBG("WEATHER", "Fetching weather from: %s", url);

  std::string response;
  if (!HttpDownloader::fetchUrl(url, response)) {
    LOG_ERR("WEATHER", "Failed to fetch weather data");
    return result;
  }

  LOG_DBG("WEATHER", "Response length: %zu", response.length());

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    LOG_ERR("WEATHER", "JSON parse error: %s", error.c_str());
    return result;
  }

  JsonObject current = doc["current"];
  JsonObject daily = doc["daily"];

  if (current.isNull() || daily.isNull()) {
    LOG_ERR("WEATHER", "Missing current or daily data");
    return result;
  }

  result.temperatureCurrent = current["temperature_2m"].as<float>();

  JsonArray maxTemps = daily["temperature_2m_max"];
  JsonArray minTemps = daily["temperature_2m_min"];
  JsonArray precipProb = daily["precipitation_probability_max"];
  JsonArray precipSum = daily["precipitation_sum"];
  JsonArray weatherCodes = daily["weather_code"];

  if (maxTemps.size() == 0 || minTemps.size() == 0) {
    LOG_ERR("WEATHER", "Missing temperature data");
    return result;
  }

  result.temperatureHigh = maxTemps[0].as<float>();
  result.temperatureLow = minTemps[0].as<float>();
  result.precipitationChance = precipProb.size() > 0 ? precipProb[0].as<int>() : 0;
  result.precipitationSum = precipSum.size() > 0 ? precipSum[0].as<float>() : 0.0f;

  int weatherCode = weatherCodes.size() > 0 ? weatherCodes[0].as<int>() : current["weather_code"].as<int>();
  result.condition = parseWeatherCode(weatherCode);

  result.valid = true;

  LOG_INF("WEATHER", "Weather: %.1f/%.1f C, %d%% precip, condition %d", result.temperatureLow, result.temperatureHigh,
          result.precipitationChance, static_cast<int>(result.condition));

  return result;
}
