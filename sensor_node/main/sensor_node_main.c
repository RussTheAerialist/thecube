#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"
#include "esp_log.h"

#ifdef CONFIG_ENABLE_DISPLAY
#include "display.h"
#endif // CONFIG_ENABLE_DISPLAY
#include "imu.h"
#include "i2c.h"
#include "mesh.h"

static const char* TAG = "sensor";

void event_loop(void* handler_arg, esp_event_base_t base, int32_t id, void* event_data)
{
  // Do Things
}

void app_main(void)
{
  nvs_flash_init();

  // Default Event Loop is created in the mesh code

  setup_i2c();
  configure_imu();
#ifdef CONFIG_ENABLE_DISPLAY
  configure_display();
#endif // CONFIG_ENABLE_DISPLAY

  configure_mesh();
  mesh_event_loop(); // replace this with above event loop

  start_imu_tasks();
}
