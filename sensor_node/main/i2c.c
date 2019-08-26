#include "driver/i2c.h"
#include "i2c.h"
#include "sdkconfig.h"

#define _GPIO(x) GPIO_NUM_##x
#define GPIO(x) _GPIO(x)
#define I2C_MASTER_SCL_IO GPIO(CONFIG_I2C_MASTER_SCL)               /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO GPIO(CONFIG_I2C_MASTER_SDA)               /*!< gpio number for I2C master data  */
#define I2C_MASTER_FREQ_HZ CONFIG_I2C_MASTER_FREQUENCY        /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0                           /*!< I2C master doesn't need buffer */

static esp_err_t master_init(int port) {
  i2c_config_t config;
  config.mode = I2C_MODE_MASTER;
  config.sda_io_num = I2C_MASTER_SDA_IO;
  config.sda_pullup_en = GPIO_PULLUP_ENABLE;
  config.scl_io_num = I2C_MASTER_SCL_IO;
  config.master.clk_speed = I2C_MASTER_FREQ_HZ;
  i2c_param_config(port, &config);
  return i2c_driver_install(port, config.mode,
      I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0); // TODO: what are these?
}

void setup_i2c() {
  master_init(I2C_MASTER_NUM);
}