#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c.h"

#define START_CMD(reg) \
		{ \
		i2c_cmd_handle_t cmd = i2c_cmd_link_create(); \
		ESP_ERROR_CHECK(i2c_master_start(cmd)); \
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_WRITE, 0x1)); \
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, reg, 0x1));

#define START_READ_CMD(reg) \
		START_CMD(reg) \
		ESP_ERROR_CHECK(i2c_master_start(cmd)); \
		ESP_ERROR_CHECK(i2c_master_write_byte(cmd, (CONFIG_IMU_ADDRESS << 1) | I2C_MASTER_READ, 0x1));

#define END_CMD \
		ESP_ERROR_CHECK(i2c_master_stop(cmd)); \
		ESP_ERROR_CHECK(i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100)); \
		i2c_cmd_link_delete(cmd); \
		}

static const char* TAG = "imu";
#define REG_CHIP_ID      0x00
#define REG_ERR          0x02
#define REG_STATUS       0x1B
#define REG_MAG_CONF     0x44
#define REG_CMD          0x7E

#define ERR_NO_ERROR     0b0000
#define ERR_ERR_1        0b0001
#define ERR_ERR_2        0b0010
#define ERR_LPW_INT      0b0011
#define ERR_RESERVED_1   0b0100
#define ERR_RESERVED_2   0b0101
#define ERR_ODR_MISMATCH 0b0110
#define ERR_PREFILT_LPW  0b0111

#define BMI160_CMD_ACC_MODE_NORMAL  0x11
#define BMI160_CMD_GYR_MODE_NORMAL  0x15
#define BMI160_CMD_SOFT_RESET       0xB6

typedef struct vec_struct {
	uint16_t x; // LSB MSB for all values
	uint16_t y;
	uint16_t z;
} vec_t;

// 24-bit resolution
typedef struct sensor_time_struct {
	uint8_t st_0; // MSB first, bytes 7-0
	uint8_t st_1; // MSB first, bytes 15-8
	uint8_t st_2; // MSB first, bytes 23-16
} sensor_time_t;

#define REG_DATA         0x04
typedef struct data_frame_struct {
	vec_t mag;
	uint16_t rhall; // TODO: What is rhall?
	vec_t gyro;
	vec_t accel;
	sensor_time_t timestamp;
} data_frame_t;
#define DATA_FRAME_SIZE (sizeof(data_frame_t))

data_frame_t read_data_frame() {
  data_frame_t retval = { 0 };
	uint8_t* data = (uint8_t *)&retval;

	START_READ_CMD(REG_DATA)
	ESP_ERROR_CHECK(i2c_master_read(cmd, data, DATA_FRAME_SIZE - 1, I2C_MASTER_ACK));
	ESP_ERROR_CHECK(i2c_master_read_byte(cmd, data + DATA_FRAME_SIZE - 1, I2C_MASTER_LAST_NACK));
	END_CMD

	ESP_LOGI(TAG, "gyro=(%d, %d, %d) accel=(%d, %d, %d)",
		retval.gyro.x, retval.gyro.y, retval.gyro.z,
		retval.accel.x, retval.accel.y, retval.accel.z
	);

	return retval;
}

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
		START_READ_CMD(reg)
		ESP_ERROR_CHECK(i2c_master_read_byte(cmd, &data, I2C_MASTER_LAST_NACK));
		END_CMD
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

void send_command(uint8_t cmd_code) {
	START_CMD(REG_CMD)
	ESP_ERROR_CHECK(i2c_master_write_byte(cmd, cmd_code, 0x1));
	END_CMD
}

void wait_for_command_to_finish() {
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	// TODO: Do the proper check
}

void configure_imu() {
	read_chip_id();
	send_command(BMI160_CMD_ACC_MODE_NORMAL);
	wait_for_command_to_finish();
	send_command(BMI160_CMD_GYR_MODE_NORMAL);
	wait_for_command_to_finish();

	// Power Up Values
	//		No Interrupts Turned on
	//		No FIFO used
	//
	// 		ACCEL set to 100 Hz, no filter defined, +-2G mode
	// 		GYRO set to 100 Hz, 3dB low pass filter, +- 2000Deg/Sec
	// 		MAG - Datasheet is wrong, need to inspect power up values - Set to RESERVED which makes me think it doesn't exist,
	//          need to check the vendor provided code.
}

void imuRead(void * parameter) {
	vTaskDelay(10000 / portTICK_PERIOD_MS); // wait 10 seconds
	while(true) {
		read_data_frame();
		vTaskDelay(1000 / portTICK_PERIOD_MS); // 100ms frame distance
	}
	vTaskDelete(NULL); // Stop the task when ready
}

void start_imu_tasks() {
	xTaskCreate(
		imuRead, "ImuRead", 10000, NULL, 1, NULL /* Task Handle? */
	);
}
