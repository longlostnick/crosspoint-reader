#include "IpGeolocation.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include <unordered_map>

#include "network/HttpDownloader.h"

static std::string abbreviateRegion(const std::string& region) {
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
  auto it = usStates.find(region);
  return it != usStates.end() ? it->second : region;
}

static std::string abbreviateCountry(const std::string& country) {
  static const std::unordered_map<std::string, std::string> countries = {
    {"United States", "USA"}, {"United Kingdom", "UK"}, {"United Arab Emirates", "UAE"},
    {"Russian Federation", "Russia"}, {"Republic of Korea", "S. Korea"}
  };
  auto it = countries.find(country);
  return it != countries.end() ? it->second : country;
}

std::string IpLocationResult::getDisplayName() const {
  std::string result = city;
  if (!region.empty() && region != city) {
    result += ", " + abbreviateRegion(region);
  }
  if (!country.empty()) {
    std::string shortCountry = abbreviateCountry(country);
    if (shortCountry != "USA" || region.empty()) {
      result += ", " + shortCountry;
    }
  }
  return result;
}

IpLocationResult IpGeolocation::getLocation() {
  IpLocationResult result;
  result.valid = false;
  
  LOG_DBG("IPGEO", "Fetching location from IP...");
  
  std::string response;
  if (!HttpDownloader::fetchUrl(API_URL, response)) {
    LOG_ERR("IPGEO", "Failed to fetch IP geolocation");
    return result;
  }
  
  LOG_DBG("IPGEO", "Response: %s", response.c_str());
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    LOG_ERR("IPGEO", "JSON parse error: %s", error.c_str());
    return result;
  }
  
  const char* status = doc["status"];
  if (!status || strcmp(status, "success") != 0) {
    LOG_ERR("IPGEO", "API returned error status");
    return result;
  }
  
  if (doc.containsKey("city")) {
    result.city = doc["city"].as<const char*>();
  }
  if (doc.containsKey("regionName")) {
    result.region = doc["regionName"].as<const char*>();
  }
  if (doc.containsKey("country")) {
    result.country = doc["country"].as<const char*>();
  }
  if (doc.containsKey("lat")) {
    result.latitude = doc["lat"].as<float>();
  }
  if (doc.containsKey("lon")) {
    result.longitude = doc["lon"].as<float>();
  }
  
  result.valid = !result.city.empty() && (result.latitude != 0.0f || result.longitude != 0.0f);
  
  if (result.valid) {
    LOG_INF("IPGEO", "Location: %s (%.4f, %.4f)", result.getDisplayName().c_str(),
            result.latitude, result.longitude);
  }
  
  return result;
}
