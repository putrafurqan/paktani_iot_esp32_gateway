idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        Gpio
                        dht22
                        i2c_tool)