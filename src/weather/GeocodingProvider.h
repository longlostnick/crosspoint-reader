#pragma once

#include <string>
#include <vector>

/**
 * Represents a geocoding search result (a location)
 */
struct GeocodingResult {
  std::string name;        // City/place name
  std::string country;     // Country name
  std::string admin1;      // State/province/region
  float latitude;
  float longitude;
  int population;          // For sorting by relevance
  
  // Get a display string like "San Francisco, California, United States"
  std::string getDisplayName() const;
};

/**
 * Abstract interface for geocoding (location search) providers.
 * Allows searching for locations by name and getting coordinates.
 */
class GeocodingProvider {
public:
  virtual ~GeocodingProvider() = default;
  
  /**
   * Search for locations matching the given query.
   * @param query The search term (city name, etc.)
   * @param maxResults Maximum number of results to return
   * @return Vector of matching locations, empty on error
   */
  virtual std::vector<GeocodingResult> search(const std::string& query, int maxResults = 5) = 0;
};
