#include "oneshot_read.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include <inttypes.h>

static const char *TAG = "ADC EXAMPLE";

static adc_cali_handle_t adc1_cali_handle;
static adc_oneshot_unit_handle_t adc1_handle;

float read_temperature()
{
    int voltage;
    float temperature;

    // Configuration structs for ADC oneshot and calibration
    adc_oneshot_unit_init_cfg_t adc1_init_cfg = {
        .unit_id = ADC_UNIT_1,
    };

    adc_oneshot_chan_cfg_t adc1_chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };

    adc_cali_line_fitting_config_t adc1_cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc1_init_cfg, &adc1_handle));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &adc1_chan_cfg));

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&adc1_cali_cfg, &adc1_cali_handle));


	int raw_val;
	// Read the ADC raw value
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_6, &raw_val));

	// Convert raw ADC reading to voltage in millivolts using calibration
	ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, raw_val, &voltage));
	ESP_LOGI(TAG, "ADC_CHANNEL_6: %d mV", voltage);

	// Calculate temperature in °C
	temperature = ((1993.0 - voltage) / 10.8333) + 10.0;
	ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);

	return temperature;
}
