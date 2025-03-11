set (SOURCES "ds3231.cpp")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES driver I2CMaster)
