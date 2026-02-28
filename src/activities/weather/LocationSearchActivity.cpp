#include "LocationSearchActivity.h"

#include <GfxRenderer.h>
#include <I18n.h>
#include <Logging.h>

#include "MappedInputManager.h"
#include "activities/util/KeyboardEntryActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"
#include "weather/OpenMeteoGeocoding.h"

void LocationSearchActivity::onEnter() {
  Activity::onEnter();
  
  state = LocationSearchState::ENTER_QUERY;
  searchQuery.clear();
  searchResults.clear();
  selectedResultIndex = 0;
  errorMessage.clear();
  
  startSearch();
}

void LocationSearchActivity::onExit() {
  Activity::onExit();
}

void LocationSearchActivity::startSearch() {
  state = LocationSearchState::ENTER_QUERY;
  
  enterNewActivity(new KeyboardEntryActivity(
      renderer, mappedInput, tr(STR_WEATHER_SEARCH_LOCATION), searchQuery, 10, 64, false,
      [this](const std::string& result) {
        searchQuery = result;
        if (!searchQuery.empty()) {
          performSearch();
        } else {
          onCancel();
        }
      },
      [this]() {
        onCancel();
      }));
}

void LocationSearchActivity::performSearch() {
  state = LocationSearchState::SEARCHING;
  requestUpdateAndWait();
  
  LOG_INF("LOCSEARCH", "Searching for: %s", searchQuery.c_str());
  
  OpenMeteoGeocoding geocoder;
  searchResults = geocoder.search(searchQuery, 10);
  
  if (searchResults.empty()) {
    state = LocationSearchState::NO_RESULTS;
    LOG_INF("LOCSEARCH", "No results found");
  } else {
    state = LocationSearchState::SHOW_RESULTS;
    selectedResultIndex = 0;
    LOG_INF("LOCSEARCH", "Found %zu results", searchResults.size());
  }
  
  requestUpdate();
}

void LocationSearchActivity::loop() {
  switch (state) {
    case LocationSearchState::SHOW_RESULTS: {
      const int numResults = static_cast<int>(searchResults.size());
      
      buttonNavigator.onNext([this, numResults] {
        selectedResultIndex = (selectedResultIndex + 1) % numResults;
        requestUpdate();
      });
      
      buttonNavigator.onPrevious([this, numResults] {
        selectedResultIndex = (selectedResultIndex - 1 + numResults) % numResults;
        requestUpdate();
      });
      
      if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
        if (selectedResultIndex >= 0 && selectedResultIndex < numResults) {
          onLocationSelected(searchResults[selectedResultIndex]);
        }
      }
      
      if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        startSearch();
      }
      break;
    }
    
    case LocationSearchState::NO_RESULTS:
    case LocationSearchState::SEARCH_ERROR:
      if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
        startSearch();
      }
      if (mappedInput.wasReleased(MappedInputManager::Button::Back)) {
        onCancel();
      }
      break;
      
    default:
      break;
  }
}

void LocationSearchActivity::render(RenderLock&&) {
  switch (state) {
    case LocationSearchState::ENTER_QUERY:
      renderEnterQuery();
      break;
    case LocationSearchState::SEARCHING:
      renderSearching();
      break;
    case LocationSearchState::SHOW_RESULTS:
      renderResults();
      break;
    case LocationSearchState::NO_RESULTS:
      renderNoResults();
      break;
    case LocationSearchState::SEARCH_ERROR:
      renderError();
      break;
  }
}

void LocationSearchActivity::renderEnterQuery() {
  // Keyboard activity handles its own rendering
}

void LocationSearchActivity::renderSearching() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_SEARCH_LOCATION));
  
  char msg[128];
  snprintf(msg, sizeof(msg), "%s \"%s\"...", tr(STR_WEATHER_SEARCHING_FOR), searchQuery.c_str());
  GUI.drawPopup(renderer, msg);
  
  renderer.displayBuffer();
}

void LocationSearchActivity::renderResults() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  char headerText[128];
  snprintf(headerText, sizeof(headerText), "%s: %s", tr(STR_WEATHER_RESULTS_FOR), searchQuery.c_str());
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, headerText);
  
  int contentY = metrics.headerHeight + metrics.verticalSpacing;
  int contentHeight = pageHeight - metrics.headerHeight - metrics.buttonHintsHeight - metrics.verticalSpacing * 2;
  
  GUI.drawList(renderer, Rect{0, contentY, pageWidth, contentHeight},
               static_cast<int>(searchResults.size()), selectedResultIndex, 0,
               [this](int index) { return searchResults[index].getDisplayName(); },
               nullptr, nullptr);
  
  const auto labels = mappedInput.mapLabels(tr(STR_BACK), tr(STR_SELECT), tr(STR_DIR_UP), tr(STR_DIR_DOWN));
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}

void LocationSearchActivity::renderNoResults() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_SEARCH_LOCATION));
  
  int centerY = pageHeight / 2;
  
  char msg[128];
  snprintf(msg, sizeof(msg), "%s \"%s\"", tr(STR_WEATHER_NO_RESULTS_FOR), searchQuery.c_str());
  renderer.drawCenteredText(UI_12_FONT_ID, centerY - 10, msg);
  renderer.drawCenteredText(UI_12_FONT_ID, centerY + 20, tr(STR_WEATHER_TRY_AGAIN));
  
  const auto labels = mappedInput.mapLabels(tr(STR_CANCEL), tr(STR_RETRY), "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}

void LocationSearchActivity::renderError() {
  const auto pageWidth = renderer.getScreenWidth();
  const auto pageHeight = renderer.getScreenHeight();
  auto metrics = UITheme::getInstance().getMetrics();
  
  renderer.clearScreen();
  
  GUI.drawHeader(renderer, Rect{0, 0, pageWidth, metrics.headerHeight}, tr(STR_WEATHER_SEARCH_LOCATION));
  
  int centerY = pageHeight / 2;
  renderer.drawCenteredText(UI_12_FONT_ID, centerY - 10, tr(STR_ERROR_MSG));
  renderer.drawCenteredText(UI_12_FONT_ID, centerY + 20, errorMessage.c_str());
  
  const auto labels = mappedInput.mapLabels(tr(STR_CANCEL), tr(STR_RETRY), "", "");
  GUI.drawButtonHints(renderer, labels.btn1, labels.btn2, labels.btn3, labels.btn4);
  
  renderer.displayBuffer();
}
