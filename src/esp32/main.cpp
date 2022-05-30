#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "plat/plat.h"
#include "emu_core.h"

#include "fabgl.h"

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

void app_main() {
  printf("Hello world!\n");

  // /* Print chip information */
  // esp_chip_info_t chip_info;
  // esp_chip_info(&chip_info);
  // printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
  //         chip_info.cores,
  //         (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
  //         (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

  //  printf("silicon revision %d, ", chip_info.revision);

  //  printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
  //         (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

  //  for (int i = 10; i >= 0; i--) {
  //      printf("Restarting in %d seconds...\n", i);
  //      vTaskDelay(1000 / portTICK_PERIOD_MS);
  //  }
  //  printf("Restarting now.\n");
  //  fflush(stdout);
  //  esp_restart();
}
}