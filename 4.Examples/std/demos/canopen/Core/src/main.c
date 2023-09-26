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

#include "master.h"
#include "slave.h"

#include "config.h"
#include "canfestival.h"

//------------------------------------------------------------------------------
//

#define cop_mst (&master_Data)
#define cop_slv (&Servo_Data)

// ＴＯＤＯ 芯片ＩＤ、编译日期　设置到０ｘ１０１８

extern void cbkSDORead(CO_Data* d, UNS8 nodeId);

int main()
{
    u32 t_blink = 0;
    u32 t_cantx = 0;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    DelayInit();

    can_init(CAN1, 1e6);
    can_init(CAN2, 1e6);

    cop_slv->canHandle = CAN1;
    setNodeId(cop_slv, 0x05);
    setState(cop_slv, Initialisation);  // 节点初始化
    setState(cop_slv, Operational);

    cop_mst->canHandle = CAN2;
    setNodeId(cop_mst, 0x00);
    setState(cop_mst, Initialisation);
    setState(cop_mst, Operational);

    while (1)
    {
        if (DelayNonBlockS(t_blink, 1))
        {
            led_tgl(LED1);

            t_blink = HAL_GetTick();
        }

        if (DelayNonBlockS(t_cantx, 1))
        {
            static int16_t counter = 0;

            ++counter;

            printf("---------------------------------\n");
            printf("%x,%x,%x,%x\n", spd_u16SpdSrc, spd_s16SpdTgt, spd_s16SpdFbk, counter);

            u8 slvid = *(cop_slv->bDeviceNodeId);

            // 以下两种方式等效
#if 1
            u32 var, size;

            var = 0x600 + slvid, size = sizeof(UNS32);
            writeLocalDict(cop_mst, 0x1280, 1, &var, &size, 0);
            var = 0x580 + slvid, size = sizeof(UNS32);
            writeLocalDict(cop_mst, 0x1280, 2, &var, &size, 0);
            var = slvid, size = sizeof(UNS8);
            writeLocalDict(cop_mst, 0x1280, 3, &var, &size, 0);

            // extern UNS32 master_obj1280_COB_ID_Client_to_Server_Transmit_SDO;
            // extern UNS32 master_obj1280_COB_ID_Server_to_Client_Receive_SDO;
            // extern UNS8  master_obj1280_Node_ID_of_the_SDO_Server;
            // printf("0x%X,0x%X,0x%X\n", master_obj1280_COB_ID_Client_to_Server_Transmit_SDO, master_obj1280_COB_ID_Server_to_Client_Receive_SDO, master_obj1280_Node_ID_of_the_SDO_Server);

#if 1  // write
            writeNetworkDict(cop_mst, slvid, 0x2000, 2, sizeof(counter), int16, &counter, 0);
#else  // read
            spd_s16SpdTgt = counter;
            readNetworkDictCallback(cop_mst, slvid, 0x2000, 2, int16, cbkSDORead, 0);
#endif

#else

#if 0  // write
            writeNetworkDictCallBackAI(cop_mst, slvid, 0x2000, 2, sizeof(counter), int16, &counter, NULL, 0, false);
#else  // read
            spd_s16SpdTgt = counter;
            readNetworkDictCallbackAI(cop_mst, slvid, 0x2000, 2, int16, cbkSDORead, 0);
#endif

#endif

            t_cantx = HAL_GetTick();
        }
    }
}

void cbkSDORead(CO_Data* d, UNS8 nodeId)
{
    UNS8  data[8] = {0};
    UNS32 size    = d->transfers[0].offset;

    u8  i;
    u32 result = 0;

    for (i = 0; i < size; ++i)
    {
        result |= d->transfers[0].data[i] << (8 * i);
    }

    printf("read sdo callback: %d\n", result);
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

    if (port == CAN1)
    {
        can_display_msg("<+> CAN1 TX", &TxMessage);
    }
    else  // port == CAN2
    {
        can_display_msg("<+> CAN2 TX", &TxMessage);
    }

    // return 0 if succes
    return CAN_Transmit((CAN_TypeDef*)port, &TxMessage) == CAN_TxStatus_NoMailBox;
}

UNS8 canChangeBaudRate(CAN_PORT port, char* baud)
{
    CAN_TypeDef* can = (CAN_TypeDef*)port;

    // enter configuare mode
    can->MCR |= CAN_MCR_INRQ;

    // set can bitrate
    // can->BTR = 0;

    // exit configuare mode
    can->MCR &= ~CAN_MCR_INRQ;

    return 0;
}

//------------------------------------------------------------------------------
// cop isr

void cop_isr(CO_Data* d)
{
    CanRxMsg RxMessage;
    Message  rxm;

    CAN_Receive(d->canHandle, CAN_FIFO0, &RxMessage);

    if (d->canHandle == CAN1)
    {
        can_display_msg("<*> CAN1 RX0", &RxMessage);
    }
    else  // if (d->canHandle == CAN2)
    {
        can_display_msg("<*> CAN2 RX0", &RxMessage);
    }

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
        // 快速 SDO 不需要反馈 (Frame-ID = 0x580 + NodeID), 即不需调用 canDispatch()

        uint8_t nodeID = rxm.cob_id & 0x7F;

        printf("SDO = %d (form 0x%X)\n", (rxm.data[5] << 8) | rxm.data[4], nodeID);

        uint8_t line = 0;

        {
            SDOCallback_t cbk;

            cbk = d->transfers[line].Callback;

            if (cbk != NULL)
            {
                uint8_t len = (rxm.data[0] - 0x40) >> 2;
                SDOtoLine(d, line, len, &(rxm.data[4]));
                (cbk)(d, nodeID);
            }
        }

        resetSDOline(d, line);
    }
    else
    {
        canDispatch(d, &rxm);
    }
}

void CAN1_RX0_IRQHandler(void)
{
    cop_isr(cop_slv);
}

void CAN2_RX0_IRQHandler(void)
{
    cop_isr(cop_mst);
}
