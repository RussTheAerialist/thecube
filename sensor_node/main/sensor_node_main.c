#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

static const char* TAG = "sensor";

void app_main(void)
{
  setup_i2c();
  configure_imu();
#ifdef CONFIG_ENABLE_DISPLAY
  configure_display();
#endif // CONFIG_ENABLE_DISPLAY

    // TODO: Replace with event loop
    for (int i = 10; i >= 0; i--) {
        ESP_LOGV(TAG, "Restarting in %d seconds...", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    ESP_LOGI(TAG, "Restarting now");
    fflush(stdout);
    esp_restart();
}
