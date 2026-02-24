#pragma once

#include <string>

/**
 * Result from IP-based geolocation lookup
 */
struct IpLocationResult {
  bool valid = false;
  std::string city;
  std::string region;
  std::string country;
  float latitude = 0.0f;
  float longitude = 0.0f;
  
  std::string getDisplayName() const;
};

/**
 * IP-based geolocation provider.
 * Uses free IP geolocation API to get approximate location.
 * Accuracy is typically city-level based on IP address.
 */
class IpGeolocation {
public:
  /**
   * Get location based on current public IP address.
   * Requires active WiFi connection.
   * @return IpLocationResult with location data, valid=false on error
   */
  static IpLocationResult getLocation();
  
private:
  static constexpr const char* API_URL = "http://ip-api.com/json/?fields=status,city,regionName,country,lat,lon";
};
