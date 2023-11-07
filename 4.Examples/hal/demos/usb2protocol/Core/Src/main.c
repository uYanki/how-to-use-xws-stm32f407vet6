/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "usbd_customhid.h"
#include "system/delay.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define CONFIG_RXBUF_SIZE 256

// 采样点推荐（CiA）：
// BPS >  800Kpbs时，sample point at 75%
// BPS >  500Kbps时，sample point at 80%
// BPS <= 500Kbps时，sample point at 87.5%

#define CAN_BTR_1000K     0x012F0001  // sjw = 2, bs1 = 16, bs2 = 3, psc = 2,   Sample Point = 85.7 %
#define CAN_BTR_500K      0x012F0003  // sjw = 2, bs1 = 16, bs2 = 3, psc = 4,   Sample Point = 85.7 %
#define CAN_BTR_250K      0x00050014  // sjw = 1, bs1 = 6,  bs2 = 1, psc = 21,  Sample Point = 87.5 %
#define CAN_BTR_125K      0x001C0014  // sjw = 1, bs1 = 13, bs2 = 2, psc = 21,  Sample Point = 87.5 %
#define CAN_BTR_50K       0x00050068  // sjw = 1, bs1 = 6,  bs2 = 1, psc = 105, Sample Point = 87.5 %
#define CAN_BTR_10K       0x0005020C  // sjw = 1, bs1 = 6,  bs2 = 1, psc = 525, Sample Point = 87.5 %

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// rs232
#define UART1_SetTxDir()
#define UART1_SetRxDir()

// rs485
#define UART2_SetTxDir() HAL_GPIO_WritePin(RS485_RTS_GPIO_Port, RS485_RTS_Pin, GPIO_PIN_RESET), DelayBlockUS(80)
#define UART2_SetRxDir() HAL_GPIO_WritePin(RS485_RTS_GPIO_Port, RS485_RTS_Pin, GPIO_PIN_SET), DelayBlockUS(80)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;
CAN_HandleTypeDef hcan2;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef  hdma_usart1_rx;
DMA_HandleTypeDef  hdma_usart1_tx;

/* USER CODE BEGIN PV */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void        SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_CAN2_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t mUart1Rxbuf[CONFIG_RXBUF_SIZE] = {0};
uint8_t mUart2Rxbuf[CONFIG_RXBUF_SIZE] = {0};

int fputc(int ch, FILE* f)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFF);
    return ch;
}

void USR_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
    // uart1: rs232
    // uart2: rs485

    if (huart->Instance == USART1)
    {
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE) == SET)
        {
            uint8_t length;

            __HAL_UART_CLEAR_IDLEFLAG(&huart1);
            HAL_UART_DMAStop(&huart1);

            length = ARRAY_SIZE(mUart1Rxbuf) - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);

            // send what recv
            // UART1_SetTxDir();
            // HAL_UART_Transmit_DMA(&huart1, mUart1Rxbuf, length);
        }
        else if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == SET)
        {
            UART1_SetRxDir();
            __HAL_DMA_SET_COUNTER(&hdma_usart1_rx, 0);
            HAL_UART_Receive_DMA(&huart1, mUart1Rxbuf, ARRAY_SIZE(mUart1Rxbuf));
        }
    }
}

void HexStr(uint8_t buff[2], uint8_t dec)
{
    uint8_t lo = dec & 0xF;
    uint8_t hi = dec >> 4;

    buff[0] = (lo < 10) ? ('0') : ('A' - 10);
    buff[1] = (hi < 10) ? ('0') : ('A' - 10);

    buff[0] += lo;
    buff[1] += hi;
}

void UploadBuffer(uint8_t* buff, uint16_t len)
{
    buff[len++] = '\r';
    buff[len++] = '\n';
    buff[len]   = '\0';

    UART1_SetTxDir();
    HAL_UART_Transmit_DMA(&huart1, buff, len);
}

void UploadCanRxMsg(CAN_RxHeaderTypeDef* hdr, uint8_t* msg)
{
    static uint8_t buff[64] = {"CAN RX: "};

    uint8_t idx = 8;
    uint8_t len = hdr->DLC;

    while (len--)
    {
        HexStr(&buff[idx], *msg);
        idx += 2;

        buff[idx] = ' ';
        idx += 1;

        msg++;
    }

    UploadBuffer(buff, idx);
}

void UploadCanTxMsg(CAN_TxHeaderTypeDef* hdr, uint8_t* msg)
{
    static uint8_t buff[64] = {"CAN TX: "};

    uint8_t idx = 8;
    uint8_t len = hdr->DLC;

    while (len--)
    {
        HexStr(&buff[idx], *msg);
        idx += 2;

        buff[idx] = ' ';
        idx += 1;

        msg++;
    }

    UploadBuffer(buff, idx);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
    // clang-format off
    CAN_RxHeaderTypeDef hdr;
    uint8_t rxMsg[8] = {0};
    // clang-format on

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hdr, rxMsg);

    if (hcan->Instance == CAN1)
    {
    }
    else if (hcan->Instance == CAN2)
    {
    }

    UploadCanRxMsg(&hdr, rxMsg);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
    // clang-format off
    CAN_RxHeaderTypeDef hdr;
    uint8_t rxMsg[8] = {0};
    // clang-format on

    HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO1, &hdr, rxMsg);

    // if (hcan == hcan1)
    if (hcan->Instance == CAN1)
    {
    }
    // else if (hcan == hcan2)
    else if (hcan->Instance == CAN2)
    {
    }

    UploadCanRxMsg(&hdr, rxMsg);
}

void UART1_Config()
{
    UART1_SetRxDir();
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE | UART_IT_TC);
    HAL_UART_Receive_DMA(&huart1, mUart1Rxbuf, ARRAY_SIZE(mUart1Rxbuf));
}
void bsp_CAN_Filt_init(void)
{
    CAN_FilterTypeDef hcan_Filt;

    hcan_Filt.FilterIdHigh         = 0x0000;
    hcan_Filt.FilterIdLow          = 0x0000;
    hcan_Filt.FilterMaskIdHigh     = 0x0000;
    hcan_Filt.FilterMaskIdLow      = 0x0000;
    hcan_Filt.FilterFIFOAssignment = CAN_FILTER_FIFO0;  // FIFO0
    hcan_Filt.FilterBank           = 0;
    hcan_Filt.FilterMode           = CAN_FILTERMODE_IDMASK;
    hcan_Filt.FilterScale          = CAN_FILTERSCALE_32BIT;
    hcan_Filt.FilterActivation     = CAN_FILTER_ENABLE;

    HAL_CAN_ConfigFilter(&hcan1, &hcan_Filt);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);  // FIFO0
    HAL_CAN_Start(&hcan1);

    hcan_Filt.FilterBank           = 14;
    hcan_Filt.SlaveStartFilterBank = 14;
    hcan_Filt.FilterFIFOAssignment = CAN_FILTER_FIFO1;  // FIFO1

    HAL_CAN_ConfigFilter(&hcan2, &hcan_Filt);
    HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);  // FIFO1
    HAL_CAN_Start(&hcan2);

    __HAL_CAN_ENABLE_IT(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    __HAL_CAN_ENABLE_IT(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);

    // HAL_CAN_Receive_IT(&hcan1, CAN_Filter_FIFO0);
    // HAL_CAN_Receive_IT(&hcan2, CAN_Filter_FIFO1);
}

void CAN1_Config(void)
{
    bsp_CAN_Filt_init();

    return;
    CAN_FilterTypeDef sFilterConfig = {
        .FilterIdHigh         = 0x0000,
        .FilterIdLow          = 0x0000,
        .FilterMaskIdHigh     = 0x0000,
        .FilterMaskIdLow      = 0x0000,
        .FilterFIFOAssignment = CAN_FILTER_FIFO0,
        .FilterBank           = 0,
        .FilterMode           = CAN_FILTERMODE_IDMASK,
        .FilterScale          = CAN_FILTERSCALE_32BIT,
        .FilterActivation     = CAN_FILTER_ENABLE,
        .SlaveStartFilterBank = 14,
    };

    // CAN1->MCR |= CAN_MCR_INRQ;
    // CAN1->BTR = CAN_BTR_1000K;
    // CAN1->MCR &= ~CAN_MCR_INRQ;

    // 0 < CAN1 FilterBank < SlaveStartFilterBank
    // SlaveStartFilterBank <= CAN2 FilterBank < 27
    HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
    // HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO1_MSG_PENDING);
    HAL_CAN_Start(&hcan1);
}

void CAN1_Test(void)
{
    static CAN_TxHeaderTypeDef hdr = {
        .StdId              = 0x700 + 0x1,
        .ExtId              = 0,
        .IDE                = CAN_ID_STD,
        .RTR                = CAN_RTR_DATA,
        .DLC                = 8,
        .TransmitGlobalTime = DISABLE,
    };

    static uint8_t txMsg[8] = {0};

    uint32_t TxMailBox;

    if (HAL_CAN_AddTxMessage(&hcan1, &hdr, txMsg, &TxMailBox) == HAL_OK)
    {
        // success
        (*(uint64_t*)&txMsg[0])++;
    }
    else
    {
        // fail
        (*(uint64_t*)&txMsg[4])++;
    }

    // while (__HAL_DMA_GET_TC_FLAG_INDEX(&hdma_usart1_tx) == 0)
    // {
    // }

    DelayBlockS(1);
    UploadCanTxMsg(&hdr, txMsg);
}

void USB1_Test(void)
{
    static uint8_t bEP1_SendBuf[64] = {0};

    if (USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, bEP1_SendBuf, 64) == USBD_OK)
    {
        bEP1_SendBuf[0]++;
    }
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();
    MX_CAN1_Init();
    MX_CAN2_Init();
    MX_I2C1_Init();
    MX_SPI2_Init();
    MX_USART2_UART_Init();
    MX_USB_DEVICE_Init();
    /* USER CODE BEGIN 2 */

    UART1_Config();
    CAN1_Config();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        USB1_Test();
        CAN1_Test();

        HAL_Delay(2000);
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 25;
    RCC_OscInitStruct.PLL.PLLN       = 336;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 7;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief CAN1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN1_Init(void)
{
    /* USER CODE BEGIN CAN1_Init 0 */

    /* USER CODE END CAN1_Init 0 */

    /* USER CODE BEGIN CAN1_Init 1 */

    /* USER CODE END CAN1_Init 1 */
    hcan1.Instance                  = CAN1;
    hcan1.Init.Prescaler            = 21;
    hcan1.Init.Mode                 = CAN_MODE_LOOPBACK;
    hcan1.Init.SyncJumpWidth        = CAN_SJW_2TQ;
    hcan1.Init.TimeSeg1             = CAN_BS1_13TQ;
    hcan1.Init.TimeSeg2             = CAN_BS2_2TQ;
    hcan1.Init.TimeTriggeredMode    = DISABLE;
    hcan1.Init.AutoBusOff           = ENABLE;
    hcan1.Init.AutoWakeUp           = DISABLE;
    hcan1.Init.AutoRetransmission   = DISABLE;
    hcan1.Init.ReceiveFifoLocked    = ENABLE;
    hcan1.Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(&hcan1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN1_Init 2 */

    /* USER CODE END CAN1_Init 2 */
}

/**
 * @brief CAN2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN2_Init(void)
{
    /* USER CODE BEGIN CAN2_Init 0 */

    /* USER CODE END CAN2_Init 0 */

    /* USER CODE BEGIN CAN2_Init 1 */

    /* USER CODE END CAN2_Init 1 */
    hcan2.Instance                  = CAN2;
    hcan2.Init.Prescaler            = 21;
    hcan2.Init.Mode                 = CAN_MODE_LOOPBACK;
    hcan2.Init.SyncJumpWidth        = CAN_SJW_2TQ;
    hcan2.Init.TimeSeg1             = CAN_BS1_13TQ;
    hcan2.Init.TimeSeg2             = CAN_BS2_2TQ;
    hcan2.Init.TimeTriggeredMode    = DISABLE;
    hcan2.Init.AutoBusOff           = ENABLE;
    hcan2.Init.AutoWakeUp           = DISABLE;
    hcan2.Init.AutoRetransmission   = DISABLE;
    hcan2.Init.ReceiveFifoLocked    = ENABLE;
    hcan2.Init.TransmitFifoPriority = ENABLE;
    if (HAL_CAN_Init(&hcan2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN CAN2_Init 2 */

    /* USER CODE END CAN2_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{
    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 100000;
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;
    hi2c1.Init.OwnAddress1     = 0;
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2     = 0;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief SPI2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI2_Init(void)
{
    /* USER CODE BEGIN SPI2_Init 0 */

    /* USER CODE END SPI2_Init 0 */

    /* USER CODE BEGIN SPI2_Init 1 */

    /* USER CODE END SPI2_Init 1 */
    /* SPI2 parameter configuration*/
    hspi2.Instance               = SPI2;
    hspi2.Init.Mode              = SPI_MODE_MASTER;
    hspi2.Init.Direction         = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize          = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity       = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase          = SPI_PHASE_1EDGE;
    hspi2.Init.NSS               = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
    hspi2.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode            = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial     = 10;
    if (HAL_SPI_Init(&hspi2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN SPI2_Init 2 */

    /* USER CODE END SPI2_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{
    /* USER CODE BEGIN USART1_Init 0 */

    /* USER CODE END USART1_Init 0 */

    /* USER CODE BEGIN USART1_Init 1 */

    /* USER CODE END USART1_Init 1 */
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART1_Init 2 */

    /* USER CODE END USART1_Init 2 */
}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{
    /* USER CODE BEGIN USART2_Init 0 */

    /* USER CODE END USART2_Init 0 */

    /* USER CODE BEGIN USART2_Init 1 */

    /* USER CODE END USART2_Init 1 */
    huart2.Instance          = USART2;
    huart2.Init.BaudRate     = 115200;
    huart2.Init.WordLength   = UART_WORDLENGTH_8B;
    huart2.Init.StopBits     = UART_STOPBITS_1;
    huart2.Init.Parity       = UART_PARITY_NONE;
    huart2.Init.Mode         = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN USART2_Init 2 */

    /* USER CODE END USART2_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA2_Stream5_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
    /* DMA2_Stream7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOE, SPI2_CS_EXFLASH_Pin | SPI2_CS_NRF24_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOE, LED1_Pin | LED2_Pin | LED3_Pin, GPIO_PIN_SET);

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(RS485_RTS_GPIO_Port, RS485_RTS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pins : SPI2_CS_EXFLASH_Pin SPI2_CS_NRF24_Pin */
    GPIO_InitStruct.Pin   = SPI2_CS_EXFLASH_Pin | SPI2_CS_NRF24_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pins : KEY1_Pin KEY2_Pin KEY3_Pin */
    GPIO_InitStruct.Pin  = KEY1_Pin | KEY2_Pin | KEY3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pins : LED1_Pin LED2_Pin LED3_Pin */
    GPIO_InitStruct.Pin   = LED1_Pin | LED2_Pin | LED3_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /*Configure GPIO pin : SD_DET_Pin */
    GPIO_InitStruct.Pin  = SD_DET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(SD_DET_GPIO_Port, &GPIO_InitStruct);

    /*Configure GPIO pin : RS485_RTS_Pin */
    GPIO_InitStruct.Pin   = RS485_RTS_Pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(RS485_RTS_GPIO_Port, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
