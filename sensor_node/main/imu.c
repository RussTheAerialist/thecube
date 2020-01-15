#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c.h"

static const char* TAG = "imu";
#define REG_CHIP_ID      0x00
#define REG_ERR          0x02
#define REG_STATUS       0x1B
#define REG_MAG_CONF     0x44

#define ERR_NO_ERROR     0b0000
#define ERR_ERR_1        0b0001
#define ERR_ERR_2        0b0010
#define ERR_LPW_INT      0b0011
#define ERR_RESERVED_1   0b0100
#define ERR_RESERVED_2   0b0101
#define ERR_ODR_MISMATCH 0b0110
#define ERR_PREFILT_LPW  0b0111

static void log_error(uint8_t err) {
	if (err & ERR_NO_ERROR)
		return;

	if (err & 0b1000) // RESERVED ERRORS
		return;

	if (err & ERR_ERR_1 || err & ERR_ERR_2) {
		ESP_LOGI(TAG, "error = generic error %02x", err); return;
	}

	if (err & ERR_LPW_INT) {
		ESP_LOGI(TAG, "error = low power mode and interrupts uses prefiltered data %02x", err); return;
	}

	if (err & ERR_ODR_MISMATCH) {
		ESP_LOGI(TAG, "error = ODRs of enabled sensors do not match %02x", err); return;
	}

	if (err & ERR_PREFILT_LPW) {
		ESP_LOGI(TAG, "error = prefiltered data are used in low power model %02x", err); return;
	}
}

static uint8_t read_register(uint8_t reg) {
	static uint8_t data = 0;
  {
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_start(cmd));
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_READ, I2C_MASTER_ACK));
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK));

		ESP_ERROR_CHECK(i2c_master_stop(cmd));
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100));
		i2c_cmd_link_delete(cmd);
	}

	return data;
}

static void read_status() {
	static uint8_t data = 0;
	data = read_register(REG_STATUS);
	ESP_LOGI(TAG, "status = %02x", data);
}

static void read_chip_id() {
	uint8_t data = read_register(REG_CHIP_ID);

	ESP_LOGI(TAG, "chip id = %02x", data);
}

void configure_imu() {
	read_chip_id();
	// Power Up Values
	//		No Interrupts Turned on
	//		No FIFO used
	//
	// 		ACCEL set to 100 Hz, no filter defined, +-2G mode
	// 		GYRO set to 100 Hz, 3dB low pass filter, +- 2000Deg/Sec
	// 		MAG - Datasheet is wrong, need to inspect power up values - Set to RESERVED which makes me think it doesn't exist,
	//          need to check the vendor provided code.
}
