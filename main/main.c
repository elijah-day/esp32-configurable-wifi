/* Copyright (C) 2023 Elijah Day

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE. */

/* TODO: Figure out which headers actually need to be included. */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "uart.h"
#include "wifi.h"
#include "net.h"

static const char *tag = "main.c";

void app_main(void)
{
	/* Create a generic return value for checking errors. */
	esp_err_t esp_err;
	
	/* Initialize NVS. */
	esp_err = nvs_flash_init();
	
	if
	(
		esp_err == ESP_ERR_NVS_NO_FREE_PAGES ||
		esp_err == ESP_ERR_NVS_NEW_VERSION_FOUND
	)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		esp_err = nvs_flash_init();
	}
	
	ESP_ERROR_CHECK(esp_err);

	/* Initialize UART and create the UART task. */
	init_uart();
	xTaskCreate
	(
		uart_event_task,
		"uart_event_task",
		UART_TASK_STACK_SIZE,
		NULL,
		UART_TASK_PRIORITY,
		NULL
	);
	
	while(1)
	{	
		switch(get_cmd_id())
		{
			case 1:
				/* Initialize WiFi, register the WiFi events. */
				init_wifi();
				register_wifi_events();
				
				/* Start the WiFi. */
				start_wifi();
				
				/*Check the Wifi bits. */
				check_wifi_bits();
				
				/* Unregister the WiFi events. */
				unregister_wifi_events();
				
				/* Start the server. */
				init_server();
		}
	
		vTaskDelay(configTICK_RATE_HZ);	
		ESP_LOGI(tag, "Tick!");
	}
	
	return;
}
