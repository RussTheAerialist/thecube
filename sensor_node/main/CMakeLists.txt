set(COMPONENT_SRCS "sensor_node_main.c" "imu.c" "i2c.c" "mesh.c")

if (CONFIG_ENABLE_DISPLAY)
  list(APPEND COMPONENT_SRCS "display.c")
endif()

idf_component_register(SRCS ${COMPONENT_SRCS} INCLUDE_DIRS "")
