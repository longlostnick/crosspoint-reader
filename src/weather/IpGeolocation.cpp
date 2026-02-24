#include "IpGeolocation.h"

#include <ArduinoJson.h>
#include <Logging.h>

#include "network/HttpDownloader.h"

std::string IpLocationResult::getDisplayName() const {
  std::string result = city;
  if (!region.empty() && region != city) {
    result += ", " + region;
  }
  if (!country.empty()) {
    result += ", " + country;
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
