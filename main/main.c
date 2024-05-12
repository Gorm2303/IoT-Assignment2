#include "wifi.h"
#include "mqtt.h"
#include "oneshot_read.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void wifi_status_handler(bool connected) {
    if (connected) {
        // Start MQTT
        mqtt_start();
    } 
}

void app_main(void) {
    init_temperature_sensor();
    
    // Initialize Wi-Fi
    wifi_register_status_callback(wifi_status_handler);
    wifi_init();
}
