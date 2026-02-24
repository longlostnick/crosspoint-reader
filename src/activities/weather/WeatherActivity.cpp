#include "WeatherActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>
#include <WiFi.h>

#include <cstdio>
#include <cstring>

#include "CrossPointSettings.h"
#include "LocationSearchActivity.h"
#include "MappedInputManager.h"
#include "activities/network/WifiSelectionActivity.h"
#include "components/UITheme.h"
#include "weather/OpenMeteoProvider.h"

void WeatherActivity::onEnter() {
  ActivityWithSubactivity::onEnter();
  
  state = WeatherActivityState::CHECKING_LOCATION;
  wifiConnected = false;
  weatherFetched = false;
  wantLocationSearch = false;
  weatherData.valid = false;
  errorMessage.clear();
  menuIndex = 0;
  
  requestUpdate();
}

void WeatherActivity::onExit() {
  ActivityWithSubactivity::onExit();
  
  disconnectWifi();
}

void WeatherActivity::disconnectWifi() {
  if (wifiConnected || WiFi.status() == WL_CONNECTED) {
    LOG_DBG("WEATHER", "Disconnecting WiFi...");
    WiFi.disconnect(false);
    delay(30);
    WiFi.mode(WIFI_OFF);
    delay(30);
    wifiConnected = false;
  }
}

void WeatherActivity::checkLocationAndProceed() {
  state = WeatherActivityState::NEEDS_WIFI;
  requestUpdate();
}

void WeatherActivity::startWifiConnection() {
  state = WeatherActivityState::WIFI_CONNECTING;
  requestUpdate();
  
  enterNewActivity(new WifiSelectionActivity(
      renderer, mappedInput,
      [this](bool connected) { onWifiComplete(connected); },
      true));
}

void WeatherActivity::onWifiComplete(bool connected) {
  wifiConnected = connected;
  
  if (connected) {
    bool hasLocation = strlen(SETTINGS.weatherLocationName) > 0 &&
                       (SETTINGS.weatherLatitude != 0.0f || SETTINGS.weatherLongitude != 0.0f);
    
    if (wantLocationSearch || !hasLocation) {
      wantLocationSearch = false;
      startLocationSearch();
    } else {
      fetchWeatherData();
    }
  } else {
    state = WeatherActivityState::DISPLAY_ERROR;
    errorMessage = tr(STR_WIFI_CONN_FAILED);
    requestUpdate();
  }
}

void WeatherActivity::startLocationSearch() {
  state = WeatherActivityState::LOCATION_SEARCH;
  requestUpdate();
  
  enterNewActivity(new LocationSearchActivity(
      renderer, mappedInput,
      [this](const GeocodingResult& location) {
        onLocationSelected(location.getDisplayName(), location.latitude, location.longitude);
      },
      [this]() {
        if (weatherFetched) {
          state = WeatherActivityState::DISPLAY_WEATHER;
        } else {
          state = WeatherActivityState::NEEDS_WIFI;
        }
        menuIndex = 0;
        requestUpdate();
      }));
}

void WeatherActivity::onLocationSelected(const std::string& name, float latitude, float longitude) {
  strncpy(SETTINGS.weatherLocationName, name.c_str(), sizeof(SETTINGS.weatherLocationName) - 1);
  SETTINGS.weatherLocationName[sizeof(SETTINGS.weatherLocationName) - 1] = '\0';
  SETTINGS.weatherLatitude = latitude;
  SETTINGS.weatherLongitude = longitude;
  SETTINGS.saveToFile();
  
  LOG_INF("WEATHER", "Location saved: %s (%.4f, %.4f)",
          SETTINGS.weatherLocationName, SETTINGS.weatherLatitude, SETTINGS.weatherLongitude);
  
  if (wifiConnected) {
    fetchWeatherData();
  } else {
    state = WeatherActivityState::NEEDS_WIFI;
    menuIndex = 0;
    requestUpdate();
  }
}

void WeatherActivity::fetchWeatherData() {
  state = WeatherActivityState::FETCHING_WEATHER;
  requestUpdateAndWait();
  
  LOG_INF("WEATHER", "Fetching weather for %s (%.4f, %.4f)",
          SETTINGS.weatherLocationName, SETTINGS.weatherLatitude, SETTINGS.weatherLongitude);
  
  OpenMeteoProvider provider;
  weatherData = provider.fetchWeather(SETTINGS.weatherLatitude, SETTINGS.weatherLongitude);
  
  if (weatherData.valid) {
    weatherData.locationName = SETTINGS.weatherLocationName;
    state = WeatherActivityState::DISPLAY_WEATHER;
    weatherFetched = true;
    LOG_INF("WEATHER", "Weather fetched successfully");
  } else {
    state = WeatherActivityState::DISPLAY_ERROR;
    errorMessage = tr(STR_WEATHER_FETCH_FAILED);
    LOG_ERR("WEATHER", "Failed to fetch weather data");
  }
  
  disconnectWifi();
  requestUpdate();
}

std::string WeatherActivity::formatTemperature(float celsius) const {
  char buf[16];
  if (SETTINGS.useFahrenheit) {
    float fahrenheit = WeatherData::toFahrenheit(celsius);
    snprintf(buf, sizeof(buf), "%.0f F", fahrenheit);
  } else {
    snprintf(buf, sizeof(buf), "%.0f C", celsius);
  }
  return buf;
}

const char* WeatherActivity::getConditionText(WeatherCondition condition) const {
  return WeatherProvider::getConditionDescription(condition);
}

void WeatherActivity::loop() {
  switch (state) {
    case WeatherActivityState::CHECKING_LOCATION:
      checkLocationAndProceed();
      break;
      
    case WeatherActivityState::NEEDS_WIFI: {
      bool hasLocation = strlen(SETTINGS.weatherLocationName) > 0 &&
                         (SETTINGS.weatherLatitude != 0.0f || SETTINGS.weatherLongitude != 0.0f);
      
      int numOptions = hasLocation ? 2 : 1;
      
      buttonNavigator.onNext([this, numOptions] {
        menuIndex = (menuIndex + 1) % numOptions;
        requestUpdate();
      });
      buttonNavigator.onPrevious([this, numOptions] {
        menuIndex = (menuIndex - 1 + numOptions) % numOptions;
        requestUpdate();
      });
      
      if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
        if (hasLocation) {
          if (menuIndex == 0) {
            // Refresh weather
            wantLocationSearch = false;
            startWifiConnection();
          } else {
            // Change location
            wantLocationSearch = true;
            startWifiConnection();
          }
        } else {
          // No location - search for one
          wantLocationSearch = true;
          startWifiConnection();
        }
      }
      if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        onGoHome();
      }
      break;
    }
      
    case WeatherActivityState::DISPLAY_WEATHER: {
      buttonNavigator.onNext([this] {
        menuIndex = (menuIndex + 1) % 2;
        requestUpdate();
      });
      buttonNavigator.onPrevious([this] {
        menuIndex = (menuIndex + 1) % 2;
        requestUpdate();
      });
      
      if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
        if (menuIndex == 0) {
          // Refresh weather
          wantLocationSearch = false;
          startWifiConnection();
        } else {
          // Change location - need WiFi for geocoding
          wantLocationSearch = true;
          startWifiConnection();
        }
      }
      if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        onGoHome();
      }
      break;
    }
      
    case WeatherActivityState::DISPLAY_ERROR:
      if (mappedInput.wasReleased(MappedInputManager::Button::Confirm) ||
          mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        onGoHome();
      }
      break;
      
    default:
      break;
  }
}

void WeatherActivity::render(Activity::RenderLock&&) {
  switch (state) {
    case WeatherActivityState::CHECKING_LOCATION:
      renderCheckingLocation();
      break;
    case WeatherActivityState::NEEDS_WIFI:
      renderNeedsWifi();
      break;
    case WeatherActivityState::FETCHING_WEATHER:
      renderFetchingWeather();
      break;
    case WeatherActivityState::DISPLAY_WEATHER:
      renderWeatherDisplay();
      break;
    case WeatherActivityState::DISPLAY_ERROR:
      renderError();
      break;
    default:
      break;
  }
}

void WeatherActivity::renderCheckingLocation() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_TITLE));
  GUI.drawPopup(renderer, tr(STR_LOADING));
  
  renderer.displayBuffer();
}

void WeatherActivity::renderNeedsWifi() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_TITLE));
  
  bool hasLocation = strlen(SETTINGS.weatherLocationName) > 0 &&
                     (SETTINGS.weatherLatitude != 0.0f || SETTINGS.weatherLongitude != 0.0f);
  
  int contentY = metrics.headerHeight + metrics.verticalSpacing;
  int contentHeight = pageHeight - metrics.headerHeight - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;
  
  if (hasLocation) {
    int textY = contentY + 10;
    renderer.setFont(UI_12_FONT_ID);
    renderer.drawTextCenteredX(pageWidth / 2, textY, tr(STR_WEATHER_LOCATION));
    textY += 25;
    renderer.setFont(UI_12_FONT_ID, EpdFontFamily::BOLD);
    renderer.drawTextCenteredX(pageWidth / 2, textY, SETTINGS.weatherLocationName);
    contentY = textY + 40;
    contentHeight -= 75;
  } else {
    int textY = contentY + 20;
    renderer.setFont(UI_12_FONT_ID);
    renderer.drawTextCenteredX(pageWidth / 2, textY, tr(STR_WEATHER_NO_LOCATION));
    contentY = textY + 40;
    contentHeight -= 60;
  }
  
  std::vector<const char*> menuItems;
  if (hasLocation) {
    menuItems = {tr(STR_WEATHER_REFRESH), tr(STR_WEATHER_CHANGE_LOCATION)};
  } else {
    menuItems = {tr(STR_WEATHER_SEARCH_LOCATION)};
  }
  
  GUI.drawButtonMenu(renderer, Rect{0, contentY, pageWidth, contentHeight},
                     static_cast<int>(menuItems.size()), menuIndex,
                     [&menuItems](int index) { return std::string(menuItems[index]); }, nullptr);
  
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}

void WeatherActivity::renderFetchingWeather() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_TITLE));
  GUI.drawPopup(renderer, tr(STR_WEATHER_FETCHING));
  
  renderer.displayBuffer();
}

void WeatherActivity::renderWeatherDisplay() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_TITLE));
  
  int contentY = metrics.headerHeight + metrics.verticalSpacing * 2;
  
  renderer.setFont(UI_12_FONT_ID);
  renderer.drawTextCenteredX(pageWidth / 2, contentY, weatherData.locationName.c_str());
  contentY += 30;
  
  renderer.setFont(UI_12_FONT_ID, EpdFontFamily::BOLD);
  const char* conditionText = getConditionText(weatherData.condition);
  renderer.drawTextCenteredX(pageWidth / 2, contentY, conditionText);
  contentY += 40;
  
  renderer.setFont(UI_12_FONT_ID);
  
  char tempLine[64];
  snprintf(tempLine, sizeof(tempLine), "%s: %s", tr(STR_WEATHER_CURRENT),
           formatTemperature(weatherData.temperatureCurrent).c_str());
  renderer.drawTextCenteredX(pageWidth / 2, contentY, tempLine);
  contentY += 30;
  
  char highLowLine[64];
  snprintf(highLowLine, sizeof(highLowLine), "%s: %s  /  %s: %s",
           tr(STR_WEATHER_HIGH), formatTemperature(weatherData.temperatureHigh).c_str(),
           tr(STR_WEATHER_LOW), formatTemperature(weatherData.temperatureLow).c_str());
  renderer.drawTextCenteredX(pageWidth / 2, contentY, highLowLine);
  contentY += 30;
  
  char precipLine[64];
  snprintf(precipLine, sizeof(precipLine), "%s: %d%%", tr(STR_WEATHER_PRECIP), weatherData.precipitationChance);
  renderer.drawTextCenteredX(pageWidth / 2, contentY, precipLine);
  contentY += 50;
  
  std::vector<const char*> menuItems = {tr(STR_WEATHER_REFRESH), tr(STR_WEATHER_CHANGE_LOCATION)};
  int menuHeight = pageHeight - contentY - metrics.buttonHintsHeight - metrics.verticalSpacing;
  
  GUI.drawButtonMenu(renderer, Rect{0, contentY, pageWidth, menuHeight},
                     static_cast<int>(menuItems.size()), menuIndex,
                     [&menuItems](int index) { return std::string(menuItems[index]); }, nullptr);
  
  const auto labels = mappedInput.mapLabels(tr(STR_HOME), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}

void WeatherActivity::renderError() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_TITLE));
  
  int centerY = pageHeight / 2;
  renderer.setFont(UI_12_FONT_ID);
  renderer.drawTextCenteredX(pageWidth / 2, centerY - 20, tr(STR_ERROR_MSG));
  renderer.drawTextCenteredX(pageWidth / 2, centerY + 10, errorMessage.c_str());
  
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), "", "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}
