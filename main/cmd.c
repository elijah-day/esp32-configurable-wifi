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
#include "esp_log.h"

#include "cmd.h"
#include "wifi.h"

static char args[CMD_ARG_MAX][CMD_ARG_SIZE];
static const char *tag = "cmd.c";

/* TODO: The int return values in this function are only temporary.  It seems
that WiFi can only be initialized in the main loop, so the function needs to
send some sort of response to notify main to init WiFi.  Try to figure out a
more standardized method. */
int handle_cmd(char *buf)
{
	int arg_cnt = 0;
	int arg_pos = 0;
	int buf_pos = 0;

	/* Delimit the arguments in this loop. */
	while(1)
	{
		if(arg_cnt >= CMD_ARG_MAX) break;
		
		if(arg_pos == CMD_ARG_SIZE)
		{
			args[arg_cnt][arg_pos] = 0;
			break;
		}
		
		/* Replace DLE or Space with NUL. */
		if(buf[buf_pos] == 10 || buf[buf_pos] == 32)
		{
			args[arg_cnt][arg_pos] = 0;
			arg_cnt++;
			arg_pos = 0;
			buf_pos++;
			continue;
		}
		
		/* Workaround for placing a space in one of the argument strings. */
		if(buf[buf_pos] == 92 && buf[buf_pos + 1] == 32)
		{
			args[arg_cnt][arg_pos] = 32;
			arg_pos++;
			buf_pos += 2;
			continue;
		}
	
		args[arg_cnt][arg_pos] = buf[buf_pos];
		
		arg_pos++;
		buf_pos++;
	}
		
	if(strcmp(args[0], "wifi-cfg") == 0)
	{	
		configure_wifi(args[1], args[2]);
	}
	
	if(strcmp(args[0], "wifi-start") == 0)
	{	
		return 1;
	}
	
	return 0;
}
