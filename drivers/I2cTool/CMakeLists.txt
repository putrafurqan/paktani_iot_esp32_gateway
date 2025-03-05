set (SOURCES "I2cTool.cpp")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES "driver")
