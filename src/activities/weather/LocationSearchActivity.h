#pragma once

#include <string>
#include <vector>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"
#include "weather/GeocodingProvider.h"

enum class LocationSearchState {
  ENTER_QUERY,      // Entering search query via keyboard
  SEARCHING,        // Searching for locations
  SHOW_RESULTS,     // Showing search results
  NO_RESULTS,       // No results found
  SEARCH_ERROR      // Error during search
};

/**
 * Activity for searching and selecting a location by name.
 * Uses geocoding API to convert city names to coordinates.
 * Returns LocationResult via setResult() when a location is selected.
 */
class LocationSearchActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  
  LocationSearchState state = LocationSearchState::ENTER_QUERY;
  std::string searchQuery;
  std::vector<GeocodingResult> searchResults;
  int selectedResultIndex = 0;
  std::string errorMessage;
  
  void startSearch();
  void performSearch();
  
  void renderEnterQuery();
  void renderSearching();
  void renderResults();
  void renderNoResults();
  void renderError();
  
public:
  explicit LocationSearchActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("LocationSearch", renderer, mappedInput) {}
  
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
