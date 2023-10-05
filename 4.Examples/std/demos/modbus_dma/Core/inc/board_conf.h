#ifndef __BOADR_CONF_H__
#define __BOADR_CONF_H__

// bsp/uart
#define CONFIG_UART_MODE            UART_MODE_RS485
#define CONFIG_UART_REDIRECT_PRINTF 0
#define CONFIG_UART_REDIRECT_SCANF  0
#define CONFIG_UART_RXCBK(buffer, length)

// system/autoinit
#define CONFIG_USDK_INIT_DEBUG 0

#endif
