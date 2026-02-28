#pragma once

#include <string>

#include "activities/Activity.h"
#include "util/ButtonNavigator.h"
#include "weather/WeatherProvider.h"

enum class WeatherActivityState {
  CHECKING_LOCATION,   // Checking if location is configured
  NEEDS_WIFI,          // Need to connect to WiFi
  WIFI_CONNECTING,     // WiFi selection in progress
  FETCHING_WEATHER,    // Fetching weather data
  DISPLAY_WEATHER,     // Showing weather info
  DISPLAY_ERROR,       // Showing error
  LOCATION_SEARCH      // Searching for location
};

class WeatherActivity final : public Activity {
  ButtonNavigator buttonNavigator;
  
  WeatherActivityState state = WeatherActivityState::CHECKING_LOCATION;
  WeatherData weatherData;
  std::string errorMessage;
  
  bool wifiConnected = false;
  bool weatherFetched = false;
  bool wantLocationSearch = false;  // True if user wants to search for location
  bool wantAutoDetect = false;      // True if user wants IP-based auto-detect
  
  int menuIndex = 0;
  
  void checkLocationAndProceed();
  void startWifiConnection();
  void onWifiComplete(bool connected);
  void fetchWeatherData();
  void disconnectWifi();
  
  void renderCheckingLocation();
  void renderNeedsWifi();
  void renderFetchingWeather();
  void renderWeatherDisplay();
  void renderError();
  
  void startLocationSearch();
  void autoDetectLocation();
  void onLocationSelected(const std::string& name, float latitude, float longitude);
  
  std::string formatTemperature(float celsius) const;
  const char* getConditionText(WeatherCondition condition) const;
  
public:
  explicit WeatherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput)
      : Activity("Weather", renderer, mappedInput) {}
  
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&&) override;
};
