#include "mqtt.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include "oneshot_read.h"


static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch (event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected successfully");
            int msg_id = esp_mqtt_client_subscribe(client, CONFIG_MQTT_COMMAND_TOPIC, 0);

            if (msg_id != -1) {
				ESP_LOGI(TAG, "Subscribed to topic '%s' successfully, message ID: %d", CONFIG_MQTT_COMMAND_TOPIC, msg_id);
			} else {
				ESP_LOGE(TAG, "Subscription to topic '%s' failed", CONFIG_MQTT_COMMAND_TOPIC);
			}

            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT disconnected");
            break;
        case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
        case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
        case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
        case MQTT_EVENT_DATA:
            // Log received data to confirm it's intact
            ESP_LOGI(TAG, "Received data: topic=%.*s, data=%.*s",
                     event->topic_len, event->topic,
                     event->data_len, event->data);

            // Initialize the parsed variables
            int repetitions = 0, delay_time_ms = 0;
            float temperature;

            // Copy the data into a null-terminated buffer
            char data_buffer[64];
            int copy_length = event->data_len < sizeof(data_buffer) - 1 ? event->data_len : sizeof(data_buffer) - 1;
            strncpy(data_buffer, event->data, copy_length);
            data_buffer[copy_length] = '\0'; // Ensure it's null-terminated

            // Update the sscanf format string to match the expected data
            if (sscanf(data_buffer, "measure:%d,%d", &repetitions, &delay_time_ms) == 2) {
                // Get the starting timestamp in microseconds
                int64_t start_time_us = esp_timer_get_time();

                // Start temperature measurement with the parsed values
                for (int i = repetitions; i > 0; i--) {
                    // Simulate temperature reading (replace with actual reading function)
                    temperature = read_temperature(); // Replace with your function

                    // Calculate elapsed time
                    int64_t current_time_us = esp_timer_get_time();
                    int elapsed_time_ms = (current_time_us - start_time_us) / 1000;

                    // Prepare the response string
                    char response[64];
                    snprintf(response, sizeof(response), "%d,%.1f,%d", i - 1, temperature, elapsed_time_ms);

                    // Publish the response to the response topic
                    esp_mqtt_client_publish(client, CONFIG_MQTT_RESPONSE_TOPIC, response, 0, 0, 0);
                    ESP_LOGI(TAG, "Publishing: %s", response);

                    // Delay before the next repetition
                    vTaskDelay(pdMS_TO_TICKS(delay_time_ms));
                }
            } else {
                // Log an error message if the input format is incorrect
                ESP_LOGE(TAG, "Invalid data format: '%s'", data_buffer);
            }

            break;

        case MQTT_EVENT_BEFORE_CONNECT:
			ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT");
			break;
        case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
			break;
        default:
            ESP_LOGI(TAG, "Unhandled MQTT event: %" PRId32, event_id);
            break;
    }
}

void mqtt_start() {
    esp_mqtt_client_config_t mqtt_cfg = {
    	.broker.address.uri = CONFIG_MQTT_BROKER, // Example: "mqtt://broker.hivemq.com"
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
