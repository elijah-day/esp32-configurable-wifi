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

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "uart.h"

static bool rx_buf_changed;
static const char *tag = "uart.c";
static QueueHandle_t uart_q;
static uint8_t *rx_buf;

static uart_config_t uart_config =
{
	/* TODO: Each of these settings, or at least the baud rate, should be
	obtained from macros. */
	.baud_rate = 115200,
	.data_bits = UART_DATA_8_BITS,
	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	.parity = UART_PARITY_DISABLE,
	.source_clk = UART_SCLK_DEFAULT,
	.stop_bits = UART_STOP_BITS_1
};

int get_uart_cmd(void)
{
	if(rx_buf_changed)
	{
		rx_buf_changed = false;
	
		if(strcmp((char *)rx_buf, "wifi"))
		{
			return 1;
		}
	}
	
	return 0;
}

void init_uart(void)
{
	rx_buf = NULL;

	/* Install the UART driver and set the configuration. */
	uart_driver_install
	(
		UART_PORT,
		RX_BUF_SIZE,
		RX_BUF_SIZE,
		UART_Q_SIZE,
		&uart_q,
		UART_INTR_ALLOC_FLAGS
	);
	
	uart_param_config(UART_PORT, &uart_config);
	
	/* Set the TX and RX pins. */
	uart_set_pin
	(
		UART_PORT,
		UART_PIN_NO_CHANGE,
		UART_PIN_NO_CHANGE,
		UART_PIN_NO_CHANGE,
		UART_PIN_NO_CHANGE
	);
}

void uart_event_task(void *parameters)
{
	uart_event_t uart_event;
	rx_buf = (uint8_t *)malloc(RX_BUF_SIZE);
	rx_buf_changed = false;

	/* Start the task loop. */
	while(1) if
	(
		xQueueReceive
		(
			uart_q,
			(void *)&uart_event,
			(TickType_t)portMAX_DELAY
		)
	)
	{
		/* Clear the RX buffer. */
		bzero(rx_buf, RX_BUF_SIZE);
		
		/* Check the type of UART event and act accordingly. */
		switch(uart_event.type)
		{
			case UART_BREAK:
				ESP_LOGI(tag, "UART_BREAK");
				break;
				
			case UART_BUFFER_FULL:
				ESP_LOGI(tag, "UART_BUFFER_FULL");
				uart_flush_input(UART_PORT);
				xQueueReset(uart_q);
				break;
				
			case UART_DATA:
				ESP_LOGI(tag, "UART_DATA");
				
				uart_read_bytes
				(
					UART_PORT,
					rx_buf,
					uart_event.size,
					portMAX_DELAY
				);
				
				rx_buf_changed = true;
				
				ESP_LOGI(tag, "DATA: %s", rx_buf);
				break;
				
			case UART_DATA_BREAK:
				ESP_LOGI(tag, "UART_DATA_BREAK");
				break;
				
			case UART_EVENT_MAX:
				ESP_LOGI(tag, "UART_EVENT_MAX");
				break;
				
			case UART_FIFO_OVF:
				ESP_LOGI(tag, "UART_FIFO_OVF");
				uart_flush_input(UART_PORT);
				xQueueReset(uart_q);
				break;
				
			case UART_FRAME_ERR:
				ESP_LOGI(tag, "UART_FRAME_ERR");
				break;
				
			case UART_PARITY_ERR:
				ESP_LOGI(tag, "UART_PARITY_ERR");
				break;
				
			case UART_PATTERN_DET:
				ESP_LOGI(tag, "UART_PATTERN_DET");
				break;
		}
	}
	
	/* Free allocated memory. */
	free(rx_buf);
	rx_buf = NULL;
	
	/* Delete the task. */
	vTaskDelete(NULL);
}
