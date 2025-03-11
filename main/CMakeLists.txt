idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES 
                        Gpio
                        Dht22
                        I2CMaster
                        Modbus
                        Wifi
                        ds3231)