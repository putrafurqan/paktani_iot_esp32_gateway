/**
 * @file dht22.h
 * @brief 
 */

#ifndef DHT_HPP
#define DHT_HPP

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2


/*
EXAMPLE USAGE:

DHT dht;
dht.setDHTgpio(DHT_GPIO);

while (1)
{
    int ret = dht.readDHT();
    dht.errorHandler(ret);

    ESP_LOGI(TAG, "Humidity: %.1f%%, Temperature: %.1f°C", dht.getHumidity(), dht.getTemperature());
}
*/

class DHT
{
  public:
	DHT();

	void setDHTgpio(gpio_num_t gpio);
	void errorHandler(int response);
	int readDHT();
	float getHumidity();
	float getTemperature();

  private:
	gpio_num_t DHTgpio;
	float humidity = 0.;
	float temperature = 0.;

	int getSignalLevel(int usTimeOut, bool state);
};

#endif
