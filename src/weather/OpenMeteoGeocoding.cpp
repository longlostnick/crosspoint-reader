#include "OpenMeteoGeocoding.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include <cstdio>

#include "network/HttpDownloader.h"

std::string GeocodingResult::getDisplayName() const {
  std::string result = name;
  if (!admin1.empty()) {
    result += ", " + admin1;
  }
  if (!country.empty()) {
    result += ", " + country;
  }
  return result;
}

static std::string urlEncode(const std::string& str) {
  std::string encoded;
  encoded.reserve(str.length() * 3);
  
  for (char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += '+';
    } else {
      char hex[4];
      snprintf(hex, sizeof(hex), "%%%02X", static_cast<unsigned char>(c));
      encoded += hex;
    }
  }
  return encoded;
}

std::vector<GeocodingResult> OpenMeteoGeocoding::search(const std::string& query, int maxResults) {
  std::vector<GeocodingResult> results;
  
  if (query.empty()) {
    return results;
  }
  
  std::string encodedQuery = urlEncode(query);
  
  char url[512];
  snprintf(url, sizeof(url), "%s?name=%s&count=%d&language=en&format=json",
           API_BASE_URL, encodedQuery.c_str(), maxResults);
  
  LOG_DBG("GEOCODE", "Searching: %s", url);
  
  std::string response;
  if (!HttpDownloader::fetchUrl(url, response)) {
    LOG_ERR("GEOCODE", "Failed to fetch geocoding data");
    return results;
  }
  
  LOG_DBG("GEOCODE", "Response length: %zu", response.length());
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    LOG_ERR("GEOCODE", "JSON parse error: %s", error.c_str());
    return results;
  }
  
  JsonArray resultsArray = doc["results"];
  if (resultsArray.isNull()) {
    LOG_DBG("GEOCODE", "No results found");
    return results;
  }
  
  for (JsonObject item : resultsArray) {
    GeocodingResult result;
    result.name = item["name"].as<const char*>();
    result.latitude = item["latitude"].as<float>();
    result.longitude = item["longitude"].as<float>();
    
    if (item.containsKey("country")) {
      result.country = item["country"].as<const char*>();
    }
    if (item.containsKey("admin1")) {
      result.admin1 = item["admin1"].as<const char*>();
    }
    if (item.containsKey("population")) {
      result.population = item["population"].as<int>();
    } else {
      result.population = 0;
    }
    
    results.push_back(result);
    LOG_DBG("GEOCODE", "Found: %s (%.4f, %.4f)", result.getDisplayName().c_str(),
            result.latitude, result.longitude);
  }
  
  LOG_INF("GEOCODE", "Found %zu results for '%s'", results.size(), query.c_str());
  return results;
}
