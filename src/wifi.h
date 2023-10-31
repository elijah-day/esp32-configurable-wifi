#ifndef WIFI_H
#define WIFI_H

#define DEFAULT_MAC 0x58, 0xAB, 0x34, 0xCD, 0x56, 0xEF
#define WIFI_CONNECTION_RETRY_MAX 10
#define WIFI_FAILURE_BIT BIT1
#define WIFI_SUCCESS_BIT BIT0

void check_wifi_bits(void);
void configure_wifi(const char *ssid, const char *password);
void register_wifi_events(void);
void start_wifi(void);
void init_wifi(void);
void unregister_wifi_events(void);

void wifi_event_handler
(
	void *event_handler_arg,
	esp_event_base_t event_base,
	int32_t event_id,
	void *event_data
);

#endif /* WIFI_H */

