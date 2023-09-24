#include "stm32f4xx.h"

#include "system/autoinit.h"
#include "system/swrst.h"
#include "system/sleep.h"

#include "bsp/key.h"
#include "bsp/led.h"
#include "bsp/rs232.h"
#include "bsp/can.h"

#include "usdk.defs.h"

#include "can_driver.h"
#include "Master.h"
#include "config.h"


#define DWT_CYCCNT       *(vu32*)0xE0001004

void TxMsg()
{
    uint8_t msg[8] = {0x40, 0x03, 0x1A, 0x01, 0x00, 0x00, 0x00, 0x00};

    CanTxMsg TxMessage = {0};

    TxMessage.StdId = 0x601;
    TxMessage.ExtId = 0;
    TxMessage.IDE   = CAN_ID_STD;
    TxMessage.RTR   = CAN_RTR_DATA;

    TxMessage.DLC = 8;

    memcpy(TxMessage.Data, msg, 8);

    can_display_msg("<-> CAN2 TX", &TxMessage);

    CAN_Transmit(CAN2, &TxMessage);

    // uint16_t reg_tst = 0x0000;
    // ++ reg_tst;
    // uint8_t msg[8] = {0x2B, 0x00, 0x20, 0x00, 0x11, 0x22, 0x00, 0x00};  // WR 2000.01 (2 bytes)
    // uint8_t msg[8] = {0x40, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};  // RD 2000.01
    // uint8_t msg[8] = {0x40, 0x03, 0x1A, 0x01, 0x00, 0x00, 0x00, 0x00};  // RD 1A03.01
}

///

#include "board_conf.h"

TIMEVAL last_counter_val = 0;
TIMEVAL elapsed_time     = 0;

// Initializes the timer, turn on the interrupt and put the interrupt time to zero
void TIM3_Init(void)
{
    NVIC_InitTypeDef        NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* TIM3 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* Enable the TIM3 gloabal Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Compute the prescaler value */
    uint16_t PrescalerValue = 1680 - 1;  // 100k(与timerscfg.h配置一致即可)，10us

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_Period        = 65535;
    TIM_TimeBaseStructure.TIM_Prescaler     = PrescalerValue;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearITPendingBit(TIM3, TIM_SR_UIF);

    /* TIM3 enable counter */  // 启动TIM3
    TIM_Cmd(TIM3, ENABLE);

    /* Preset counter for a safe start */
    TIM_SetCounter(TIM3, 1);

    /* TIM Interrupts enable */
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
}

// Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{
    printf("[%d]\n", value);
    uint32_t timer = TIM_GetCounter(TIM3);  // Copy the value of the running timer
    elapsed_time += timer - last_counter_val;
    last_counter_val = 65535 - value;
    TIM_SetCounter(TIM3, 65535 - value);
    TIM_Cmd(TIM3, ENABLE);
    // printf("setTimer %lu, elapsed %lu\r\n", value, elapsed_time);
}

// Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
    uint32_t timer = TIM_GetCounter(TIM3);  // Copy the value of the running timer
    if (timer < last_counter_val)
    {
        timer += 65535;
    }
    TIMEVAL elapsed = timer - last_counter_val + elapsed_time;
    // printf("elapsed %lu - %lu %lu %lu\r\n", elapsed, timer, last_counter_val, elapsed_time);
    printf(".");
    return elapsed;
}

// This function handles Timer 3 interrupt request.
void TIM3_IRQHandler(void)
{
    if (TIM_GetFlagStatus(TIM3, TIM_SR_UIF) == RESET)
    {
        return;
    }
    last_counter_val = 0;
    elapsed_time     = 0;
    TIM_ClearITPendingBit(TIM3, TIM_SR_UIF);
			 printf("---------- %d ----------\n", DWT_CYCCNT );
    TimeDispatch();
	
}



int main()
{
    u32 blink = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    DelayInit();

    can_init(CAN1, 1e6);
    can_init(CAN2, 1e6);

    void TIM3_Init();
    TIM3_Init();

    setNodeId(&Master_Data, 0x01);
    setState(&Master_Data, Initialisation);  // 节点初始化
    setState(&Master_Data, Operational);

    while (1)
    {

        if (DelayNonBlockS(blink, 1))
        {
            led_tgl(LED1);
            blink = HAL_GetTick();
            TxMsg();

        }
    }
}

//

void usdk_hw_uart_init(void)
{
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    rs232_init(&USART_InitStructure);
}

//------------------------------------------------------------------------------
//

void CAN2_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
    can_display_msg("<-> CAN2 RX0", &RxMessage);
}

#include "canfestival.h"
#include "master.h"

UNS8 canSend(CAN_PORT port, Message* m)
{
    CanTxMsg TxMessage;

    TxMessage.StdId = m->cob_id;
    TxMessage.ExtId = 0;
    TxMessage.IDE   = CAN_ID_STD;
    TxMessage.RTR   = m->rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    TxMessage.DLC   = m->len;
    memcpy(TxMessage.Data, m->data, m->len);

    can_display_msg("<+> CAN1 TX", &TxMessage);

    return CAN_Transmit(CAN1, &TxMessage) != CAN_TxStatus_NoMailBox;
}

UNS8 canChangeBaudRate(CAN_PORT port, char* baud)
{
    return 0;
}

///

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    can_display_msg("<*> CAN1 RX0", &RxMessage);

    Message rxm;

    if (RxMessage.IDE == CAN_ID_EXT)
    {
        // Drop extended frames
        return;
    }

    rxm.cob_id = RxMessage.StdId;
    rxm.rtr    = RxMessage.RTR == CAN_RTR_REMOTE;
    rxm.len    = RxMessage.DLC;
    memcpy(rxm.data, RxMessage.Data, rxm.len);

    if (rxm.cob_id >> 7 == 0xB)  // 快速SDO回应ID为0x580+对方id, 右移7位即为0xB
    {
        int16_t test = 0;
        switch (rxm.cob_id)
        {
            case 0x582:
                test = rxm.data[4] | rxm.data[5] << 8;
                printf("test=%d,\r\n", test);
                break;
            default:
                break;
        }
    }
    else
    {
        canDispatch(&Master_Data, &rxm);  // CANopen自身的处理函数，因为快速SDO不需要反馈，所以在上边处理后就不需要调用这步了
    }
}
