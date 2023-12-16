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
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>

#include "esp_log.h"
#include "net.h"

static char buf[SOCKET_BUF_SIZE];
static const char *tag = "net.c";
static int client_socket;
static int rv; /* Generic return value integer. */
static int server_socket;
static socklen_t address_len;
static struct sockaddr_in address;

/* TODO: Most of this code REALLY needs to be cleaned up (made more
readable). */
void init_server(void)
{	
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	if(server_socket < 0)
	{
		ESP_LOGI(tag, "Unable to create server socket!\n");
		return;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PORT);
	address_len = sizeof(address);
	
	rv = bind(server_socket, (struct sockaddr *)&address, address_len);
	if(rv < 0)
	{
		ESP_LOGI(tag, "Unable to bind server socket!\n");
		return;
	}

	listen(server_socket, 2);
	
	client_socket = accept
	(
		server_socket,
		(struct sockaddr *)&address,
		&address_len
	);
	
	read(client_socket, buf, SOCKET_BUF_SIZE - 1);
	ESP_LOGI(tag, "%s\n", buf);
	send(client_socket, "h\n", 3, 0);
	
	ESP_LOGI(tag, "CONTACT MADE!!\n");

	close(client_socket);
	close(server_socket);
}
