#include "OpenMeteoGeocoding.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include <cstdio>
#include <unordered_map>

#include "network/HttpDownloader.h"

static std::string abbreviateState(const std::string& state) {
  static const std::unordered_map<std::string, std::string> usStates = {
    {"Alabama", "AL"}, {"Alaska", "AK"}, {"Arizona", "AZ"}, {"Arkansas", "AR"},
    {"California", "CA"}, {"Colorado", "CO"}, {"Connecticut", "CT"}, {"Delaware", "DE"},
    {"Florida", "FL"}, {"Georgia", "GA"}, {"Hawaii", "HI"}, {"Idaho", "ID"},
    {"Illinois", "IL"}, {"Indiana", "IN"}, {"Iowa", "IA"}, {"Kansas", "KS"},
    {"Kentucky", "KY"}, {"Louisiana", "LA"}, {"Maine", "ME"}, {"Maryland", "MD"},
    {"Massachusetts", "MA"}, {"Michigan", "MI"}, {"Minnesota", "MN"}, {"Mississippi", "MS"},
    {"Missouri", "MO"}, {"Montana", "MT"}, {"Nebraska", "NE"}, {"Nevada", "NV"},
    {"New Hampshire", "NH"}, {"New Jersey", "NJ"}, {"New Mexico", "NM"}, {"New York", "NY"},
    {"North Carolina", "NC"}, {"North Dakota", "ND"}, {"Ohio", "OH"}, {"Oklahoma", "OK"},
    {"Oregon", "OR"}, {"Pennsylvania", "PA"}, {"Rhode Island", "RI"}, {"South Carolina", "SC"},
    {"South Dakota", "SD"}, {"Tennessee", "TN"}, {"Texas", "TX"}, {"Utah", "UT"},
    {"Vermont", "VT"}, {"Virginia", "VA"}, {"Washington", "WA"}, {"West Virginia", "WV"},
    {"Wisconsin", "WI"}, {"Wyoming", "WY"}, {"District of Columbia", "DC"}
  };
  auto it = usStates.find(state);
  return it != usStates.end() ? it->second : state;
}

static std::string abbreviateCountry(const std::string& country) {
  static const std::unordered_map<std::string, std::string> countries = {
    {"United States", "USA"}, {"United Kingdom", "UK"}, {"United Arab Emirates", "UAE"},
    {"Russian Federation", "Russia"}, {"Republic of Korea", "S. Korea"},
    {"Democratic People's Republic of Korea", "N. Korea"}
  };
  auto it = countries.find(country);
  return it != countries.end() ? it->second : country;
}

std::string GeocodingResult::getDisplayName() const {
  std::string result = name;
  if (!admin1.empty()) {
    result += ", " + abbreviateState(admin1);
  }
  if (!country.empty()) {
    std::string shortCountry = abbreviateCountry(country);
    if (shortCountry != "USA" || admin1.empty()) {
      result += ", " + shortCountry;
    }
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
