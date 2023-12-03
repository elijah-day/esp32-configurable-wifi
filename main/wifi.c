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

#include "wifi.h"

static const char *tag = "wifi.c";
static const uint8_t mac[] = {DEFAULT_MAC}; /* TODO: Make this non-static. */
static EventGroupHandle_t wifi_event_group;
static esp_event_handler_instance_t ip_event;
static esp_event_handler_instance_t wifi_event;
static int wifi_connection_retry_cnt;
static int wifi_status;

static wifi_config_t wifi_config =
{
	.sta =
	{
		.password = "",
		.pmf_cfg =
		{
			.capable = true,
			.required = false
		},
		.ssid = "",
		.threshold.authmode = WIFI_AUTH_WPA2_PSK,
	}
};

void check_wifi_bits(void)
{
	/* Wait for the event bits. */
	EventBits_t wifi_event_bits = xEventGroupWaitBits
	(
		wifi_event_group,
		WIFI_FAILURE_BIT | WIFI_SUCCESS_BIT,
		pdFALSE,
		pdFALSE,
		portMAX_DELAY
	);
	
	/* Check the event bits. */
	if(wifi_event_bits & WIFI_SUCCESS_BIT)
	{
		ESP_LOGI(tag, "WiFi successfully connected!!");
		wifi_status = 1;
	}
	else if(wifi_event_bits & WIFI_FAILURE_BIT)
	{
		ESP_LOGI(tag, "WiFi failed to connect!!");
		wifi_status = 0;
	}
	else
	{
		ESP_LOGI(tag, "WiFi failed to connect!!");
		wifi_status = 0;
	}
}

void configure_wifi(const char *ssid, const char *password)
{
	strcpy((char *)wifi_config.sta.password, password);
	strcpy((char *)wifi_config.sta.ssid, ssid);
	
	ESP_LOGI(tag, "New SSID: \"%s\"", wifi_config.sta.ssid);
	ESP_LOGI(tag, "New password: \"%s\"", wifi_config.sta.password);
}

void register_wifi_events(void)
{
	/* Create the WiFi event group handler. */	
	wifi_event_group = xEventGroupCreate();
	
	/* Register the event handler instances. */
	ESP_ERROR_CHECK
	(
		esp_event_handler_instance_register
		(
			WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			&wifi_event_handler,
			NULL,
			&wifi_event
		)
	);
	
	ESP_ERROR_CHECK
	(
		esp_event_handler_instance_register
		(
			IP_EVENT,
			IP_EVENT_STA_GOT_IP,
			&wifi_event_handler,
			NULL,
			&ip_event
		)
	);
}

void start_wifi(void)
{
	/* TODO: Make this configure dynamically via UART. */
	
	/* Set the WiFi controller settings. */
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	
	/* Start the WiFi driver. */
	ESP_ERROR_CHECK(esp_wifi_start());
	
	/* Set the MAC address. */
	ESP_ERROR_CHECK(esp_wifi_set_mac(ESP_IF_WIFI_STA, mac));
}

void init_wifi(void)
{
	/* Set initial values. */
	wifi_connection_retry_cnt = 0;
	wifi_status = 0;

	/* Initialize the "underlying TCP/IP stack" and create the default event
	loop. */
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	
	/* Start default WiFi initialization. */
	esp_netif_create_default_wifi_sta();
	
	/* Initialize ESP WiFi with the default configuration. */
	wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
}

void unregister_wifi_events(void)
{
	/* Unregister the handler instances. */
	ESP_ERROR_CHECK
	(
		esp_event_handler_instance_unregister
		(
			WIFI_EVENT,
			ESP_EVENT_ANY_ID,
			wifi_event
		)
	);
	
	ESP_ERROR_CHECK
	(
		esp_event_handler_instance_unregister
		(
			IP_EVENT,
			IP_EVENT_STA_GOT_IP,
			ip_event
		)
	);
	
	/* Delete the WiFi event group. */
	vEventGroupDelete(wifi_event_group);
}

void wifi_event_handler
(
	void *event_handler_arg,
	esp_event_base_t event_base,
	int32_t event_id,
	void *event_data
)
{
	if(event_base == IP_EVENT)
	{	
		if(event_id == IP_EVENT_STA_GOT_IP)
		{
			ip_event_got_ip_t *ip_event_got_ip =
			(ip_event_got_ip_t *)event_data;
			
			ESP_LOGI
			(
				tag,
				"Got IP: " IPSTR,
				IP2STR(&ip_event_got_ip->ip_info.ip)
			);
			
			xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS_BIT);
		}
	}
	else if(event_base == WIFI_EVENT)
	{
		if(event_id == WIFI_EVENT_STA_START)
		{
			esp_wifi_connect();
		}
		else if(event_id == WIFI_EVENT_STA_DISCONNECTED)
		{
			if(wifi_connection_retry_cnt < WIFI_CONNECTION_RETRY_MAX)
			{
				esp_wifi_connect();
				wifi_connection_retry_cnt += 1;
			}
			else
			{
				xEventGroupSetBits(wifi_event_group, WIFI_FAILURE_BIT);
			}
		}
	}
}
