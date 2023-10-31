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

static const char *tag = "main.c";
static int cmd_id;

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
		cmd_id = 0;
		cmd_id = get_uart_cmd();
		
		switch(cmd_id)
		{
			case 1:
				/* Initialize WiFi, register the WiFi events. */
				configure_wifi("SSID", "Password");
				init_wifi();
				register_wifi_events();
				
				/* Start the WiFi. */
				start_wifi();
				
				/*Check the Wifi bits. */
				check_wifi_bits();
				
				/* Unregister the WiFi events. */
				unregister_wifi_events();
				break;
		}
		
		vTaskDelay(configTICK_RATE_HZ / 4);	
		ESP_LOGI(tag, "Tick!");
	}
	
	return;
}
