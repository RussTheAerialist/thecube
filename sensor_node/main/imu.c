#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c.h"

static const char* TAG = "imu";

static void read_chip_id() {
	uint8_t data = 0;
	{
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_READ, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK));

		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100));
		i2c_cmd_link_delete(cmd);
	}

	// {
	// 	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	// 	ESP_ERROR_CHECK(i2c_master_start(cmd));
	// 	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_READ, I2C_MASTER_ACK));
	// 	ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK));
	// 	ESP_ERROR_CHECK(i2c_master_stop(cmd));
	// 	ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100));
	// 	i2c_cmd_link_delete(cmd);
	// }

	ESP_LOGI(TAG, "chip id = %02x", data);
}

void configure_imu() {
	read_chip_id();
}
