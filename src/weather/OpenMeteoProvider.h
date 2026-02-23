#pragma once

#include "WeatherProvider.h"

/**
 * Weather provider implementation using the Open-Meteo API.
 * Open-Meteo is a free, open-source weather API that doesn't require an API key.
 * https://open-meteo.com/
 */
class OpenMeteoProvider : public WeatherProvider {
public:
  OpenMeteoProvider() = default;
  ~OpenMeteoProvider() override = default;
  
  WeatherData fetchWeather(float latitude, float longitude) override;
  
private:
  static constexpr const char* API_BASE_URL = "https://api.open-meteo.com/v1/forecast";
};
