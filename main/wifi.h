#include <stdbool.h>
#ifndef WIFI_H
#define WIFI_H

// Define a callback type for WiFi events
typedef void (*wifi_status_callback_t)(bool connected);

void wifi_register_status_callback(wifi_status_callback_t callback);

void wifi_init(void);

#endif
