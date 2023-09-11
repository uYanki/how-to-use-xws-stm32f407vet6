#ifndef __PIN_MAP_H__
#define __PIN_MAP_H__

#include "stm32f4xx.h"

#define LED1_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define LED1_GPIO_PORT            GPIOE
#define LED1_GPIO_PIN             GPIO_Pin_13

#define LED2_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define LED2_GPIO_PORT            GPIOE
#define LED2_GPIO_PIN             GPIO_Pin_14

#define LED3_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define LED3_GPIO_PORT            GPIOE
#define LED3_GPIO_PIN             GPIO_Pin_15

#define LED_ON(port, pin)         GPIO_WriteBit(port, pin, Bit_RESET)  // 低有效
#define LED_OFF(port, pin)        GPIO_WriteBit(port, pin, Bit_SET)
#define LED_TGL(port, pin)        GPIO_WriteBit(!GPIO_ReadInputDataBit(port, pin))

//

#define KEY1_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define KEY1_GPIO_PORT            GPIOE
#define KEY1_GPIO_PIN             GPIO_Pin_10

#define KEY2_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define KEY2_GPIO_PORT            GPIOE
#define KEY2_GPIO_PIN             GPIO_Pin_11

#define KEY3_GPIO_CLK             RCC_AHB1Periph_GPIOE
#define KEY3_GPIO_PORT            GPIOE
#define KEY3_GPIO_PIN             GPIO_Pin_12

#define KEY_IS_RELEASE(port, pin) ((BitAction)GPIO_ReadInputDataBit(port, pin) == Bit_SET)    // 默认上拉
#define KEY_IS_PRESS(port, pin)   ((BitAction)GPIO_ReadInputDataBit(port, pin) == Bit_RESET)  // 低有效

//

#define DS18B20_DQ_GPIO_PORT      GPIOE
#define DS18B20_DQ_GPIO_PIN       GPIO_Pin_2

//

#define RS232_TX_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define RS232_TX_GPIO_PORT        GPIOA
#define RS232_TX_GPIO_PIN         GPIO_Pin_9

#define RS232_RX_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define RS232_RX_GPIO_PORT        GPIOA
#define RS232_RX_GPIO_PIN         GPIO_Pin_10

#define RS232_UART_CLKEN(...)     RCC_APB2PeriphClockCmd(...)
#define RS232_UART_CLK            RCC_APB2Periph_USART1
#define RS232_UART_PORT           USART1

//

#define RS485_TX_GPIO_CLK         RCC_AHB1Periph_GPIOD
#define RS485_TX_GPIO_PORT        GPIOD
#define RS485_TX_GPIO_PIN         GPIO_Pin_5

#define RS485_RX_GPIO_CLK         RCC_AHB1Periph_GPIOD
#define RS485_RX_GPIO_PORT        GPIOD
#define RS485_RX_GPIO_PIN         GPIO_Pin_6

#define RS485_RTS_GPIO_CLK        RCC_AHB1Periph_GPIOD
#define RS485_RTS_GPIO_PORT       GPIOD
#define RS485_RTS_GPIO_PIN        GPIO_Pin_7

#define RS485_UART_CLKEN(...)     RCC_APB1PeriphClockCmd(...)
#define RS485_UART_CLK            RCC_APB1Periph_USART2
#define RS485_UART_PORT           USART2

#define RS485_SetTxDir()          GPIO_WriteBit(RS485_RTS_GPIO_PORT, RS485_RTS_GPIO_PIN, Bit_RESET)
#define RS485_SetRxDir()          GPIO_WriteBit(RS485_RTS_GPIO_PORT, RS485_RTS_GPIO_PIN, Bit_SET)

//

#define CAN1_TX_GPIO_PORT         GPIOD
#define CAN1_TX_GPIO_PIN          GPIO_Pin_1
#define CAN1_RX_GPIO_PORT         GPIOD
#define CAN1_RX_GPIO_PIN          GPIO_Pin_0

#define CAN2_TX_GPIO_PORT         GPIOB
#define CAN2_TX_GPIO_PIN          GPIO_Pin_6
#define CAN2_RX_GPIO_PORT         GPIOB
#define CAN2_RX_GPIO_PIN          GPIO_Pin_5

//

#define AT24C02_SCL_GPIO_PORT     GPIOB
#define AT24C02_SCL_GPIO_PIN      GPIO_Pin_8

#define AT24C02_SDA_GPIO_PORT     GPIOB
#define AT24C02_SDA_GPIO_PIN      GPIO_Pin_9

#define AT24C02_I2C_PORT          I2C1

//

#define W25Q128_SCLK_GPIO_PORT    GPIOB
#define W25Q128_SCLK_GPIO_PIN     GPIO_Pin_10

#define W25Q128_MOSI_GPIO_PORT    GPIOC
#define W25Q128_MOSI_GPIO_PIN     GPIO_Pin_3

#define W25Q128_MISO_GPIO_PORT    GPIOE
#define W25Q128_MISO_GPIO_PIN     GPIO_Pin_2

#define W25Q128_CS_GPIO_PORT      GPIOE
#define W25Q128_CS_GPIO_PIN       GPIO_Pin_3

#define W25Q128_SPI_PORT          SPI2
#define W25Q128_SPI_CLKEN(...)    RCC_APB1PeriphClockCmd(...)
#define W25Q128_SPI_CLK           RCC_APB1Periph_SPI2

//

#define NRF24L01_SCLK_GPIO_PORT   GPIOB
#define NRF24L01_SCLK_GPIO_PIN    GPIO_Pin_10

#define NRF24L01_MOSI_GPIO_PORT   GPIOC
#define NRF24L01_MOSI_GPIO_PIN    GPIO_Pin_3

#define NRF24L01_MISO_GPIO_PORT   GPIOE
#define NRF24L01_MISO_GPIO_PIN    GPIO_Pin_2

#define NRF24L01_CE_GPIO_PORT     GPIOE
#define NRF24L01_CE_GPIO_PIN      GPIO_Pin_7

#define NRF24L01_CS_GPIO_PORT     GPIOE
#define NRF24L01_CS_GPIO_PIN      GPIO_Pin_8

#define NRF24L01_IRQ_GPIO_PORT    GPIOE
#define NRF24L01_IRQ_GPIO_PIN     GPIO_Pin_9

#define NRF24L01_SPI_CLKEN(...)   RCC_APB1PeriphClockCmd(...)
#define NRF24L01_SPI_CLK          RCC_APB1Periph_SPI2
#define NRF24L01_SPI_PORT         SPI2

//

#define USB1_DP_GPIO_PORT         GPIOA
#define USB1_DP_GPIO_PIN          GPIO_Pin_12

#define USB1_DM_GPIO_PORT         GPIOA
#define USB1_DM_GPIO_PIN          GPIO_Pin_11

#define USB2_DP_GPIO_PORT         GPIOB
#define USB2_DP_GPIO_PIN          GPIO_Pin_15

#define USB2_DM_GPIO_PORT         GPIOB
#define USB2_DM_GPIO_PIN          GPIO_Pin_14

//

#define TFCARD_DAT0_GPIO_PORT     GPIOC
#define TFCARD_DAT0_GPIO_PIN      GPIO_Pin_8

#define TFCARD_DAT1_GPIO_PORT     GPIOC
#define TFCARD_DAT1_GPIO_PIN      GPIO_Pin_9

#define TFCARD_DAT2_GPIO_PORT     GPIOC
#define TFCARD_DAT2_GPIO_PIN      GPIO_Pin_10

#define TFCARD_DAT3_GPIO_PORT     GPIOC
#define TFCARD_DAT3_GPIO_PIN      GPIO_Pin_11

#define TFCARD_CLK_GPIO_PORT      GPIOC
#define TFCARD_CLK_GPIO_PIN       GPIO_Pin_12

#define TFCARD_CMD_GPIO_PORT      GPIOD
#define TFCARD_CMD_GPIO_PIN       GPIO_Pin_2

#define TFCARD_DET_GPIO_PORT      GPIOD
#define TFCARD_DET_GPIO_PIN       GPIO_Pin_3

#define TFCARD_SPI  // SDIO

#define TFCARD_IS_EXIST(port, pin) ((BitAction)GPIO_ReadInputDataBit(port, pin) == Bit_RESET)  // 默认上拉, 低有效

#endif