#pragma once

#include "GeocodingProvider.h"

/**
 * Geocoding provider using the Open-Meteo Geocoding API.
 * Free to use, no API key required.
 * https://open-meteo.com/en/docs/geocoding-api
 */
class OpenMeteoGeocoding : public GeocodingProvider {
public:
  OpenMeteoGeocoding() = default;
  ~OpenMeteoGeocoding() override = default;
  
  std::vector<GeocodingResult> search(const std::string& query, int maxResults = 5) override;
  
private:
  static constexpr const char* API_BASE_URL = "https://geocoding-api.open-meteo.com/v1/search";
};
