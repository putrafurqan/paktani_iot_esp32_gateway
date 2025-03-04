set (SOURCES "i2c_tool.cpp")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES "driver")
