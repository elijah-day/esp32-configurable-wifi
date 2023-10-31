#ifndef UART_H
#define UART_H

#define RX_BUF_SIZE 1024
#define UART_INTR_ALLOC_FLAGS 0
#define UART_PORT UART_NUM_0
#define UART_Q_SIZE 10
#define UART_TASK_PRIORITY 12
#define UART_TASK_STACK_SIZE 2048

int get_uart_cmd(void);
void init_uart(void);
void uart_event_task(void *parameters);

#endif /* UART_H */

