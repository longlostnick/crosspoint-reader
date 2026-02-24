#pragma once

/**
 * SD Card OTA Update functionality.
 * Checks for a firmware.bin file on the SD card and flashes it if found.
 * 
 * Usage:
 * 1. Upload firmware.bin to the root of the SD card via web interface
 * 2. Reboot the device
 * 3. Device will detect firmware.bin, flash it, rename to firmware.bin.done, and reboot
 * 
 * The file is renamed after successful flash to prevent boot loops.
 */
class SdCardOta {
public:
  static constexpr const char* FIRMWARE_PATH = "/firmware.bin";
  static constexpr const char* FIRMWARE_DONE_PATH = "/firmware.bin.done";
  
  /**
   * Check if a firmware update file exists on SD card.
   * @return true if /firmware.bin exists
   */
  static bool hasUpdate();
  
  /**
   * Install the firmware update from SD card.
   * This function will:
   * 1. Read firmware.bin from SD card
   * 2. Flash it to the OTA partition
   * 3. Rename firmware.bin to firmware.bin.done
   * 4. Reboot the device
   * 
   * @return false if installation failed (will not return on success due to reboot)
   */
  static bool installUpdate();
};
