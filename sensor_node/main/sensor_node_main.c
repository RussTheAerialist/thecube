#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"
#include "esp_log.h"

#ifdef CONFIG_ENABLE_DISPLAY
#include "display.h"
#endif // CONFIG_ENABLE_DISPLAY
#include "imu.h"
#include "i2c.h"
#include "mesh.h"

static const char* TAG = "sensor";

void display_boot_header() {
  const esp_partition_t *running = esp_ota_get_running_partition();

  esp_app_desc_t running_app_info;
  if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
      ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
  }
}

void app_main(void)
{
  nvs_flash_init();
  display_boot_header();

  // Default Event Loop is created in the mesh code

  setup_i2c();
  configure_imu();
#ifdef CONFIG_ENABLE_DISPLAY
  configure_display();
#endif // CONFIG_ENABLE_DISPLAY

  // configure_mesh();
  // mesh_event_loop(); // replace this with above event loop

  start_imu_tasks();
}
