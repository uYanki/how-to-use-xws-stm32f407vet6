

#define CONFIG_LOGIC_PROG          0

//---------------------------------------------------
//

// dclk_o
#define O_DCLK_PORT                GPIOA
#define O_DCLK_PIN                 GPIO_Pin_4

// data_o
#define O_DATA_PORT                GPIOC
#define O_DATA_PIN                 GPIO_Pin_5

// n_config_o
#define O_CONFIG_PORT              GPIOA
#define O_CONFIG_PIN               GPIO_Pin_5

// n_status_i
#define I_STATUS_PORT              GPIOB
#define I_STATUS_PIN               GPIO_Pin_0

// conf_done_i
#define I_CONF_DONE_PORT           GPIOC
#define I_CONF_DONE_PIN            GPIO_Pin_4

// rst_o
#define O_RST_PORT                 GPIOC
#define O_RST_PIN                  GPIO_Pin_8

//

#define GPIO_SET(port, pin, level) GPIO_WriteBit(port, pin, (level) ? Bit_SET : Bit_RESET)
#define GPIO_GET(port, pin)        GPIO_ReadInputDataBit(port, pin)

#define SET_DCLK_H()               GPIO_SET(O_DCLK_PORT, O_DCLK_PIN, 1)
#define SET_DCLK_L()               GPIO_SET(O_DCLK_PORT, O_DCLK_PIN, 0)

#define SET_DATA_H()               GPIO_SET(O_DATA_PORT, O_DATA_PIN, 1)
#define SET_DATA_L()               GPIO_SET(O_DATA_PORT, O_DATA_PIN, 0)

#define SET_RST_H()                GPIO_SET(O_RST_PORT, O_RST_PIN, 1)
#define SET_RST_L()                GPIO_SET(O_RST_PORT, O_RST_PIN, 0)

#define SET_CONFIG_H()             GPIO_SET(O_CONFIG_PORT, O_CONFIG_PIN, 1)
#define SET_CONFIG_L()             GPIO_SET(O_CONFIG_PORT, O_CONFIG_PIN, 0)

#define GET_STATUS_X()             GPIO_GET(I_STATUS_PORT, I_STATUS_PIN)
#define GET_CONF_DONE_X()          GPIO_GET(I_CONF_DONE_PORT, I_CONF_DONE_PIN)

#if CONFIG_LOGIC_PROG

__force_inline void GenerateClockPulse(void)
{
    SET_DCLK_H();
    __nop();
    __nop();
    __nop();
    __nop();

    SET_DCLK_L();
    __nop();
    __nop();
    __nop();
    __nop();
}

__force_inline void GPIO_ShiftOut(u8 data)
{
    u8 n = 8;

    while (n--)
    {
#if 1  // LSB_FIRST: EP4

        (data & 0x01) ? SET_DATA_H() : SET_DATA_L();

        data >>= 1;

#else  // MSB_FIRST: lattice

        (data & 0x80) ? SET_DATA_H() : SET_DATA_L();

        data <<= 1;

#endif

        GenerateClockPulse();
    }
}

static int BSP_Logic_Loader_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);

    // input

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;

    GPIO_InitStructure.GPIO_Pin = I_STATUS_PIN;
    GPIO_Init(I_STATUS_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = I_CONF_DONE_PIN;
    GPIO_Init(I_CONF_DONE_PORT, &GPIO_InitStructure);

    // output

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = O_CONFIG_PIN;
    GPIO_Init(O_CONFIG_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(O_CONFIG_PORT, O_CONFIG_PIN, Bit_SET);

    GPIO_InitStructure.GPIO_Pin = O_DCLK_PIN;
    GPIO_Init(O_DCLK_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(O_DCLK_PORT, O_DCLK_PIN, Bit_RESET);

    GPIO_InitStructure.GPIO_Pin = O_DATA_PIN;
    GPIO_Init(O_DATA_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(O_DATA_PORT, O_DATA_PIN, Bit_RESET);

    GPIO_InitStructure.GPIO_Pin = O_RST_PIN;
    GPIO_Init(O_RST_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(O_RST_PORT, O_RST_PIN, Bit_SET);

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(BSP_Logic_Loader_Init, INIT_LEVEL_BOARD)

static int Logic_Loader_Prog(void)
{
    u8 retries = 5;

    u16 timeout;
    u8  pulse;

    u32 index;

_sof:

    while (retries--)
    {
        SET_DCLK_L();
        SET_CONFIG_L();
        SET_DCLK_L();

        sleep_us(2);
        SET_CONFIG_H();

        timeout = 0;

        while (++timeout)
        {
            __nop();
            __nop();

            if (GET_STATUS_X())
            {
                // success
                goto _prog;  // ↓
            }
        }
    }

    return INIT_RESULT_FAILURE;

_prog:

    sleep_us(30);

    // burn it

    for (index = 0; index < ARRAY_SIZE(fpga_data); ++index)
    {
        GPIO_ShiftOut(fpga_data[index]);
    }

    // check status

    SET_DATA_L();

    timeout = 0;

    while (++timeout)
    {
        // provide data clock pulse

        pulse = GET_CONF_DONE_X() ? 2 : 10;

        while (pulse--)
        {
            GenerateClockPulse();
        }

        if (GET_CONF_DONE_X())
        {
            // success
            goto _eof;  // ↓
        }
    }

    // fail, reprog
    goto _sof;  // ↑

_eof:

    sleep_us(710);
    SET_RST_L();
    sleep_ms(20);
    SET_RST_H();

    return INIT_RESULT_SUCCESS;
}

USDK_INIT_EXPORT(Logic_Loader_Prog, INIT_LEVEL_DEVICE)

#endif