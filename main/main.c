#include "wifi.h"
#include "mqtt.h"

void app_main(void) {
    // Initialize Wi-Fi
	wifi_init();

    // Start MQTT
    mqtt_start();
}
