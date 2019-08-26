#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c.h"

static const char* TAG = "imu";

static void read_chip_id() {
	uint8_t data = 0;
	esp_err_t err;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_WRITE, true));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));

	ESP_ERROR_CHECK(i2c_master_start(cmd));
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_READ, true));
	ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &data, false));
	ESP_ERROR_CHECK(i2c_master_stop(cmd));

	err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Error reading chip id for IMU: %s", esp_err_to_name(err));
	}

	i2c_cmd_link_delete(cmd);

}

void configure_imu() {
	read_chip_id();
}
