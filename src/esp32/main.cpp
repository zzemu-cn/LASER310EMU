#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// #include "esp_system.h"
// #include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"

#include "plat/plat.h"

#include "gbldefs.h"
#include "esp32/esp32_gblvar.h"
#include "emu_core.h"

#include "FileIO.h"

#include "fabgl.h"

#define CHARROM_SIZE  0x0C00
#define SYSROM_SIZE   0x4000
#define DOSROM_SIZE   0x2000

uint8_t	fontrom[CHARROM_SIZE];
uint8_t sysrom[SYSROM_SIZE];
uint8_t	dosrom[DOSROM_SIZE];

fabgl::VGADirectController DisplayController;

extern "C" {

void initDisplay() {
  /* VGA control */
  DisplayController.begin();
  //DisplayController.setDrawScanlineCallback(drawScanline);
  DisplayController.setResolution(VGA_320x200_75Hz);    //VGA_640x480_60Hz  
}

void startEmulator() {
  emu_exit = 0;

	/* Start the emulator, really. */
//	emu_setScreenUpdateCallback(UpdateScreen);
//	thMain = thread_create(emu_thread, &quited);
//	SetThreadPriority(thMain, THREAD_PRIORITY_HIGHEST);
  
  printf("Emulator started.");
}

static const char *TAG = "Laser310Emu";

bool setup()
{
  uint8_t *tmpFileBuffer = NULL;
  unsigned long int fileSize = 0;

  ESP_LOGI(TAG, "Initializing SPIFFS");

  esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 5,
    .format_if_mount_failed = false
  };

  esp_err_t ret = esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
      if (ret == ESP_FAIL) {
          ESP_LOGE(TAG, "Failed to mount or format filesystem");
      } else if (ret == ESP_ERR_NOT_FOUND) {
          ESP_LOGE(TAG, "Failed to find SPIFFS partition");
      } else {
          ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
      }
      return false;
  }  

  ESP_LOGI(TAG, "SPIFFS partition mounted.");

  tmpFileBuffer = LoadRomFile("/spiffs/character_set.rom", &fileSize);
  if (!tmpFileBuffer || fileSize != sizeof(fontrom)) {
    ESP_LOGE(TAG, "Failed to load character_set.rom");
    return false;
  }
	memcpy(fontrom, tmpFileBuffer, CHARROM_SIZE);

	tmpFileBuffer = LoadRomFile("/spiffs/basic_v2.0.rom", &fileSize);
  if (!tmpFileBuffer || fileSize != sizeof(sysrom)) {
    ESP_LOGE(TAG, "Failed to load basic_v2.0.rom");
    return false;
  }
	memcpy(sysrom, tmpFileBuffer, SYSROM_SIZE);

	tmpFileBuffer = LoadRomFile("/spiffs/dos_basic_v1.2_patched.rom", &fileSize);
  if (!tmpFileBuffer || fileSize != sizeof(dosrom)) {
    ESP_LOGE(TAG, "Failed to load dos_basic_v1.2_patched.rom");
    return false;
  }  
	memcpy(dosrom, tmpFileBuffer, DOSROM_SIZE);

  return true;
}

void app_main()
{
  if (setup() == true) {
    ESP_LOGI(TAG, "LAER310EMU started.");
  }
  else {
    ESP_LOGE(TAG, "Failed to initialize LASER310EMU."); 
  }
}
}