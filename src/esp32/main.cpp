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
#include "utils/prgdef.h"
#include "esp32/esp32_gblvar.h"
#include "emu.h"
#include "emu_core.h"

#include "FileIO.h"

#include "fabgl.h"

uint8_t	*fontrom = NULL;
uint8_t *sysrom = NULL;
uint8_t	*dosrom = NULL;

fabgl::VGADirectController DisplayController;

extern "C" {

void initDisplay() {
  /* VGA control */
  DisplayController.begin();
  //DisplayController.setDrawScanlineCallback(drawScanline);
  DisplayController.setResolution(VGA_320x200_75Hz);    //VGA_640x480_60Hz  
}

void UpdateScreen() {

}

void startEmulator() {
  int emu_exit = 0;

	/* Start the emulator, really. */
  emu_setScreenUpdateCallback(UpdateScreen);
//	thMain = thread_create(emu_thread, &emu_exit);
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

	if (!tmp_buf) {
		tmp_buf = (uint8_t *)malloc(TMP_BUF_LEN);
	}

  tmpFileBuffer = LoadRomFile("/spiffs/character_set.rom", &fileSize);
  fontrom = (uint8_t *)malloc(CHARROM_SIZE);  
  if (tmpFileBuffer && fontrom) {
      memcpy(fontrom, tmpFileBuffer, CHARROM_SIZE);
  } else {
    ESP_LOGE(TAG, "Failed to load character_set.rom");
    return false;
  }

	tmpFileBuffer = LoadRomFile("/spiffs/basic_v2.0.rom", &fileSize);
  sysrom = (uint8_t *)malloc(SYSROM_SIZE);  
  if (tmpFileBuffer && sysrom) {
	  memcpy(sysrom, tmpFileBuffer, SYSROM_SIZE);
  } else {    
    ESP_LOGE(TAG, "Failed to load basic_v2.0.rom");
    return false;
  }

	tmpFileBuffer = LoadRomFile("/spiffs/dos_basic_v1.2_patched.rom", &fileSize);
  dosrom = (uint8_t *)malloc(DOSROM_SIZE);  
  if (tmpFileBuffer && dosrom) {
	  memcpy(dosrom, tmpFileBuffer, DOSROM_SIZE);
  } else {    
    ESP_LOGE(TAG, "Failed to load dos_basic_v1.2_patched.rom");
    return false;
  }

  return true;
}

void teardown()
{
	if (tmp_buf) {
		free(tmp_buf);
		tmp_buf = NULL;
	}
}

void app_main()
{
  if (setup() == true) {
    ESP_LOGI(TAG, "LAER310EMU started.");

    EmulationInitialize(fontrom, sysrom, dosrom);
    startEmulator();

    if (fontrom) {
      free(fontrom);
      fontrom = NULL;
    }

    if (sysrom) {
      free(sysrom);
      sysrom = NULL;
    }

    if (dosrom) {
      free(dosrom);
      dosrom = NULL;
    }

    for(;;) {
      //Main loop
    }
  }
  else {
    ESP_LOGE(TAG, "Failed to initialize LASER310EMU."); 
  }

  teardown();
}
}