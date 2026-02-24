#include "SdCardOta.h"

#include <HalStorage.h>
#include <Logging.h>
#include <esp_ota_ops.h>
#include <esp_system.h>

bool SdCardOta::hasUpdate() {
  return Storage.exists(FIRMWARE_PATH);
}

bool SdCardOta::installUpdate() {
  if (!hasUpdate()) {
    LOG_DBG("SDOTA", "No firmware.bin found on SD card");
    return false;
  }
  
  LOG_INF("SDOTA", "Found firmware.bin on SD card, starting update...");
  
  FsFile file;
  if (!Storage.openFileForRead("SDOTA", FIRMWARE_PATH, file)) {
    LOG_ERR("SDOTA", "Failed to open firmware.bin");
    return false;
  }
  
  size_t fileSize = file.size();
  if (fileSize == 0) {
    LOG_ERR("SDOTA", "firmware.bin is empty");
    file.close();
    return false;
  }
  
  LOG_INF("SDOTA", "Firmware size: %zu bytes", fileSize);
  
  const esp_partition_t* updatePartition = esp_ota_get_next_update_partition(nullptr);
  if (updatePartition == nullptr) {
    LOG_ERR("SDOTA", "Failed to get update partition");
    file.close();
    return false;
  }
  
  LOG_INF("SDOTA", "Writing to partition: %s (offset 0x%x, size %d)",
          updatePartition->label, updatePartition->address, updatePartition->size);
  
  if (fileSize > updatePartition->size) {
    LOG_ERR("SDOTA", "Firmware too large for partition");
    file.close();
    return false;
  }
  
  esp_ota_handle_t otaHandle;
  esp_err_t err = esp_ota_begin(updatePartition, fileSize, &otaHandle);
  if (err != ESP_OK) {
    LOG_ERR("SDOTA", "esp_ota_begin failed: %s", esp_err_to_name(err));
    file.close();
    return false;
  }
  
  constexpr size_t BUFFER_SIZE = 4096;
  uint8_t* buffer = static_cast<uint8_t*>(malloc(BUFFER_SIZE));
  if (buffer == nullptr) {
    LOG_ERR("SDOTA", "Failed to allocate buffer");
    esp_ota_abort(otaHandle);
    file.close();
    return false;
  }
  
  size_t written = 0;
  while (written < fileSize) {
    size_t toRead = min(BUFFER_SIZE, fileSize - written);
    size_t bytesRead = file.read(buffer, toRead);
    
    if (bytesRead == 0) {
      LOG_ERR("SDOTA", "Read error at offset %zu", written);
      free(buffer);
      esp_ota_abort(otaHandle);
      file.close();
      return false;
    }
    
    err = esp_ota_write(otaHandle, buffer, bytesRead);
    if (err != ESP_OK) {
      LOG_ERR("SDOTA", "esp_ota_write failed: %s", esp_err_to_name(err));
      free(buffer);
      esp_ota_abort(otaHandle);
      file.close();
      return false;
    }
    
    written += bytesRead;
    
    if (written % (BUFFER_SIZE * 10) == 0 || written == fileSize) {
      LOG_INF("SDOTA", "Progress: %zu / %zu bytes (%d%%)", 
              written, fileSize, static_cast<int>(written * 100 / fileSize));
    }
  }
  
  free(buffer);
  file.close();
  
  err = esp_ota_end(otaHandle);
  if (err != ESP_OK) {
    LOG_ERR("SDOTA", "esp_ota_end failed: %s", esp_err_to_name(err));
    return false;
  }
  
  err = esp_ota_set_boot_partition(updatePartition);
  if (err != ESP_OK) {
    LOG_ERR("SDOTA", "esp_ota_set_boot_partition failed: %s", esp_err_to_name(err));
    return false;
  }
  
  LOG_INF("SDOTA", "Update successful! Renaming firmware.bin...");
  
  if (Storage.exists(FIRMWARE_DONE_PATH)) {
    Storage.remove(FIRMWARE_DONE_PATH);
  }
  Storage.rename(FIRMWARE_PATH, FIRMWARE_DONE_PATH);
  
  LOG_INF("SDOTA", "Rebooting...");
  delay(100);
  esp_restart();
  
  return true;
}
