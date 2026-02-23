#pragma once

#include <functional>
#include <string>

#include "activities/ActivityWithSubactivity.h"
#include "util/ButtonNavigator.h"
#include "weather/WeatherProvider.h"

enum class WeatherActivityState {
  CHECKING_LOCATION,   // Checking if location is configured
  NEEDS_WIFI,          // Need to connect to WiFi
  WIFI_CONNECTING,     // WiFi selection in progress
  FETCHING_WEATHER,    // Fetching weather data
  DISPLAY_WEATHER,     // Showing weather info
  DISPLAY_ERROR,       // Showing error
  LOCATION_SETUP,      // Setting up location
  LOCATION_NAME_ENTRY, // Entering location name
  LATITUDE_ENTRY,      // Entering latitude
  LONGITUDE_ENTRY      // Entering longitude
};

class WeatherActivity final : public ActivityWithSubactivity {
  ButtonNavigator buttonNavigator;
  
  WeatherActivityState state = WeatherActivityState::CHECKING_LOCATION;
  WeatherData weatherData;
  std::string errorMessage;
  
  const std::function<void()> onGoHome;
  
  bool wifiConnected = false;
  bool weatherFetched = false;
  
  int menuIndex = 0;
  
  std::string pendingLocationName;
  std::string pendingLatitude;
  std::string pendingLongitude;
  
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
  void renderLocationSetup();
  
  void startLocationSetup();
  void saveLocationSettings();
  
  std::string formatTemperature(float celsius) const;
  const char* getConditionText(WeatherCondition condition) const;
  
public:
  explicit WeatherActivity(GfxRenderer& renderer, MappedInputManager& mappedInput,
                           const std::function<void()>& onGoHome)
      : ActivityWithSubactivity("Weather", renderer, mappedInput), onGoHome(onGoHome) {}
  
  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(Activity::RenderLock&&) override;
};
