#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_app_format.h"
#include "esp_ota_ops.h"

void print_startup_info() {
  const esp_app_desc_t* description = esp_ota_get_app_description();

  printf("Bungee Sensor v%s\n", description->version);
}


void app_main(void)
{
  print_startup_info();

    // TODO: Replace with event loop
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
