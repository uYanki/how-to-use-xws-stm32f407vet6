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
#include "objdict.h"
#include "config.h"
#include "canfestival.h"

//------------------------------------------------------------------------------
//

#define cop_slv &Servo_Data

void CAN2_TxMsg(void);

int main()
{
    u32 t_blink = 0;
    u32 t_cantx = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    DelayInit();

    can_init(CAN1, 1e6);
    can_init(CAN2, 1e6);

    setNodeId(cop_slv, 0x01);
    setState(cop_slv, Initialisation);  // 节点初始化
    setState(cop_slv, Operational);

    while (1)
    {
        if (DelayNonBlockS(t_blink, 1))
        {
            led_tgl(LED1);

            // __disable_irq();
            printf("%x,%x,%x\n", spd_u16SpdSrc, spd_s16SpdTgt, spd_s16SpdFbk);
            printf("---------------------\n");
            // __enable_irq();

            CAN2_TxMsg();

            t_blink = HAL_GetTick();
        }

				
				// ＴＯＤＯ 芯片ＩＤ、编译日期　设置到０ｘ１０１８
				
#if 0
				

        if (DelayNonBlockS(t_cantx, 1))
        {
            {
                UNS32 value = 0;
                UNS32 value_size;
                UNS8  type;
                UNS32 ret = getODentry(cop_slv,
                                       0x2000,
                                       0,
                                       (void*)&value,
                                       (unsigned long*)&value_size,
                                       (UNS8*)&type,
                                       RW);
                printf("value = %u\r\n", value);
            }

            {
                s16 value = 1;
                s16 size  = 2;

                value++;

                setODentry(cop_slv,
                           0x2000,
                           2,
                           (void*)&value,
                           (unsigned long*)&size,
                           RW);
            }

            t_cantx = HAL_GetTick();
        }

#endif
    }
}

//------------------------------------------------------------------------------
// usdk

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
// cop tim

TIMEVAL m_cop_timeout = 0;

void setTimer(TIMEVAL value)
{
    m_cop_timeout = HAL_GetTick() + value;
}

TIMEVAL getElapsedTime(void)
{
    static TIMEVAL last = TICK_MAX;

    TIMEVAL cur = HAL_GetTick();

    TIMEVAL ret = HAL_DeltaTick(last, cur);

    last = cur;

    return ret;
}

void $Sub$$SysTick_Handler(void)
{
    extern void $Super$$SysTick_Handler(void);
    $Super$$SysTick_Handler();

    // COP: TIM_IRQHandler
    if (HAL_GetTick() > m_cop_timeout)
    {
        TimeDispatch();
    }
}

//------------------------------------------------------------------------------
// cop can

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

void CAN1_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    Message  rxm;

    CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
    can_display_msg("<*> CAN1 RX0", &RxMessage);

    if (RxMessage.IDE == CAN_ID_EXT)
    {
        // Drop extended frames（忽略扩展帧）
        return;
    }

    rxm.cob_id = RxMessage.StdId;
    rxm.rtr    = RxMessage.RTR == CAN_RTR_REMOTE;
    rxm.len    = RxMessage.DLC;
    memcpy(rxm.data, RxMessage.Data, rxm.len);

    if ((rxm.cob_id >> 7) == 0xB)
    {
        // 快速 SDO 不需要反馈, Frame-ID = 0x580 + NodeID

        switch (rxm.cob_id & 0x7F)
        {
            case 0x02:
                printf("SDO = %d \n", rxm.data[5] << 8 | rxm.data[4]);
                break;
            default:
                break;
        }
    }
    else
    {
        canDispatch(cop_slv, &rxm);
    }
}

//------------------------------------------------------------------------------
//

void CAN2_RX0_IRQHandler(void)
{
    CanRxMsg RxMessage;
    CAN_Receive(CAN2, CAN_FIFO0, &RxMessage);
    can_display_msg("<-> CAN2 RX0", &RxMessage);
}

void CAN2_TxMsg(void)
{
    static int16_t value = 0x0000;

    uint8_t msg[8]   = {0x2B, 0x00, 0x20, 0x02, 0x11, 0x22, 0x00, 0x00};  // WR 2000.01 (2 bytes)
    *((s16*)&msg[4]) = value++;

    // uint8_t msg[8] = {0x40, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00};  // RD 2000.01
    // uint8_t msg[8] = {0x40, 0x03, 0x1A, 0x01, 0x00, 0x00, 0x00, 0x00};  // RD 1A03.01

    CanTxMsg TxMessage;

    TxMessage.StdId = 0x600 + *((cop_slv)->bDeviceNodeId);
    TxMessage.ExtId = 0;
    TxMessage.IDE   = CAN_ID_STD;
    TxMessage.RTR   = CAN_RTR_DATA;

    TxMessage.DLC = 8;

    memcpy(TxMessage.Data, msg, 8);

    can_display_msg("<-> CAN2 TX", &TxMessage);

    CAN_Transmit(CAN2, &TxMessage);
}
