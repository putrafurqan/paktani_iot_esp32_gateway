set (SOURCES "Dht22.cpp")

idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS "."
                       REQUIRES "driver")
