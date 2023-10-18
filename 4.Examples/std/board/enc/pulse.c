#include "pulse/calc.h"
#include "pulse.h"

//---------------------------------------------------------
// GPIO

#define PULSE_GEN_MST_GPIO_CLKEN     RCC_AHB1PeriphClockCmd
#define PULSE_GEN_MST_GPIO_CLK       RCC_AHB1Periph_GPIOA
#define PULSE_GEN_MST_GPIO_AF        GPIO_AF_TIM2

//---------------------------------------------------------
// Master TIM

#define PULSE_GEN_MST_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_MST_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_MST_TIM_CLK        RCC_APB1Periph_TIM2
#define PULSE_GEN_MST_TIM_PORT       TIM2

//---------------------------------------------------------
// Slave TIM

#define PULSE_GEN_SLV_TIM_FRQ        84000000ul  // 84M (APB1)
#define PULSE_GEN_SLV_TIM_CLKEN      RCC_APB1PeriphClockCmd
#define PULSE_GEN_SLV_TIM_CLK        RCC_APB1Periph_TIM3
#define PULSE_GEN_SLV_TIM_PORT       TIM3
#define PULSE_GEN_SLV_TIM_ITR        TIM_TS_ITR1
#define PULSE_GEN_SLV_TIM_IRQn       TIM3_IRQn
#define PULSE_GEN_SLV_TIM_IRQHandler TIM3_IRQHandler

//---------------------------------------------------------
// TIM Channel

#define PULSE_GEN_MST_TIM_CH         4  // <--- config

#if PULSE_GEN_MST_TIM_CH == 1  // CH1: PA0

#define PULSE_GEN_MST_GPIO_PORT   GPIOA
#define PULSE_GEN_MST_GPIO_PIN    GPIO_Pin_0
#define PULSE_GEN_MST_GPIO_PINSRC GPIO_PinSource0

#define PULSE_GEN_SLV_TIM_TRGSRC  TIM_TRGOSource_OC1Ref
#define PULSE_GEN_SLV_TIM_IT      TIM_IT_CC1

#elif PULSE_GEN_MST_TIM_CH == 4  // CH4: PA3

#define PULSE_GEN_MST_GPIO_PORT   GPIOA
#define PULSE_GEN_MST_GPIO_PIN    GPIO_Pin_3
#define PULSE_GEN_MST_GPIO_PINSRC GPIO_PinSource3

#define PULSE_GEN_SLV_TIM_TRGSRC  TIM_TRGOSource_OC4Ref
#define PULSE_GEN_SLV_TIM_IT      TIM_IT_CC4

#else

#error "please select a pulse generate channel"

#endif

//---------------------------------------------------------

/**
 * @note ʱ��Ƶ��
 *
 *      - APB1_GPIO_CLK: 42M
 *      - APB1_TIM_CLK: 84M
 *      - APB2_GPIO_CLK: 84M
 *      - APB2_TIM_CLK: 168M
 *
 * @note ʱ�ӷ�Ƶ
 *
 *      - TIM_Prescaler: clksrc = internal (APBx)
 *      - TIM_ClockDivision: clksrc = external, �ⲿʱ���˲�ʱ�� (TIM1ETR..)
 *
 * @note ����ģʽ
 *
 *      - TIM_CounterMode:
 *           ���ض���ģʽ: ���������ݷ���λ����������ݼ�������
 *              TIM_CounterMode_Up:   0->TIM_Period
 *              TIM_CounterMode_Down: TIM_Period->0
 *           ���Ķ���ģʽ: ������������е��������͵ݼ�����
 *              TIM_CounterMode_CenterAligned1: 0->TIM_Period->0, ���ݼ�ʱ,������Ƚ��ж�  (TIM_IT_CC)
 *              TIM_CounterMode_CenterAligned2: 0->TIM_Period->0, ���ݼ�ʱ,������Ƚ��ж�
 *              TIM_CounterMode_CenterAligned3: 0->TIM_Period->0, �ݼ�����,������Ƚ��ж�
 *           ע:ֻҪ����������ʹ��״̬, �Ͳ��ôӱ��ضԲ�ģʽ�л�Ϊ��С�Բ�ģ
 *
 * @note �ظ�����
 *
 *      - TIM_RepetitionCounter: �ڶ�ʱ���ظ����n�μ�����, ��������ж�, �������¼� (TIM_IT_Update)
 *           ����? ����ADC������DAC���...
 *
 * @note ��Ч��ƽ
 *
 *    ���ϼ���ʱ:
 *
 *      - TIM_OCMode = TIM_OCMode_PWM1:
 *
 *          TIMx_CNT < TIMx_CCRn, �����Ч��ƽ
 *          TIMx_CNT > TIMx_CCRn, �����Ч��ƽ
 *
 *      - TIM_OCMode = TIM_OCMode_PWM2:
 *
 *          TIMx_CNT < TIMx_CCRn, �����Ч��ƽ
 *          TIMx_CNT > TIMx_CCRn, �����Ч��ƽ
 *
 * @note ��Ч��ƽ
 *
 *    - TIM_OCPolarity = TIM_OCPolarity_High���ߵ�ƽ��
 *    - TIM_OCPolarity = TIM_OCPolarity_Low ���͵�ƽ��
 *
 * @note ����Դ
 *
 *      Trigger Source: TI1FP1, TI1(Timer input 1) after FP(filter and polarity selection)
 *
 * @note ������ģʽ (One Pulse)
 *
 *     TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Single);  // one pulse mode
 *     TIM_SelectOnePulseMode(PULSE_GEN_TIM_PORT, TIM_OPMode_Repetitive);  // pwm mode
 *
 * @note �Զ���װ��Ԥװ������λ (Auto-reload preload enable, ARPE)
 *
 *      Disable���Զ���װ�ؼĴ���д����ֵ�󣬼����������������������Ȼ��ʼ�µļ�������
 *      Enable�� �Զ���װ�ؼĴ���д����ֵ�󣬼�������ɵ�ǰ�ɵļ������ٿ�ʼ�µļ�������
 */

void PULSE_GEN_SLV_TIM_IRQHandler()  // �ﵽָ��������
{
    if (SET == TIM_GetITStatus(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT))
    {
        TIM_ClearITPendingBit(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT);
        PulseGenStop();
    }
}

void PulseGenInit(void)
{
    // gpio
    {
        GPIO_InitTypeDef GPIO_InitStructure = {
            .GPIO_Pin   = PULSE_GEN_MST_GPIO_PIN,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP,
        };

        PULSE_GEN_MST_GPIO_CLKEN(PULSE_GEN_MST_GPIO_CLK, ENABLE);
        GPIO_PinAFConfig(PULSE_GEN_MST_GPIO_PORT, PULSE_GEN_MST_GPIO_PINSRC, PULSE_GEN_MST_GPIO_AF);
        GPIO_Init(PULSE_GEN_MST_GPIO_PORT, &GPIO_InitStructure);
    }
    // mst tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
        TIM_OCInitTypeDef       TIM_OCInitStructure;

        // clkin
        PULSE_GEN_MST_TIM_CLKEN(PULSE_GEN_MST_TIM_CLK, ENABLE);
        TIM_InternalClockConfig(PULSE_GEN_MST_TIM_PORT);

        // timer
        TIM_TimeBaseStructure.TIM_Prescaler         = 84 - 1;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period            = 100 - 1;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(PULSE_GEN_MST_TIM_PORT, &TIM_TimeBaseStructure);
        TIM_ARRPreloadConfig(PULSE_GEN_MST_TIM_PORT, DISABLE);  // disable ARPE

        // pwm
        TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
        TIM_OCInitStructure.TIM_Pulse       = (TIM_TimeBaseStructure.TIM_Period + 1) * 0.33 - 1;  // duty
        TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;

#if (PULSE_GEN_MST_TIM_CH == 1)
        TIM_OC1Init(PULSE_GEN_MST_TIM_PORT, &TIM_OCInitStructure);
        TIM_OC1PreloadConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCPreload_Disable);
        TIM_OC1FastConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCFast_Disable);
#elif (PULSE_GEN_MST_TIM_CH == 4)
        TIM_OC4Init(PULSE_GEN_MST_TIM_PORT, &TIM_OCInitStructure);
        TIM_OC4PreloadConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCPreload_Disable);
        TIM_OC4FastConfig(PULSE_GEN_MST_TIM_PORT, TIM_OCFast_Disable);
#endif

        // event
        TIM_SelectMasterSlaveMode(PULSE_GEN_MST_TIM_PORT, TIM_MasterSlaveMode_Enable);
        TIM_SelectOutputTrigger(PULSE_GEN_MST_TIM_PORT, PULSE_GEN_SLV_TIM_TRGSRC);
    }
    // slv tim
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

        // clkin
        PULSE_GEN_SLV_TIM_CLKEN(PULSE_GEN_SLV_TIM_CLK, ENABLE);

        // timer
        TIM_TimeBaseStructure.TIM_Prescaler         = 0;
        TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
        TIM_TimeBaseStructure.TIM_Period            = 0;
        TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
        TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
        TIM_TimeBaseInit(PULSE_GEN_SLV_TIM_PORT, &TIM_TimeBaseStructure);

        // event
        TIM_SelectSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_SlaveMode_External1);
        TIM_SelectInputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_ITR);
        TIM_SelectMasterSlaveMode(PULSE_GEN_SLV_TIM_PORT, TIM_MasterSlaveMode_Disable);
        TIM_SelectOutputTrigger(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_TRGSRC);

        // nvic
        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel                   = PULSE_GEN_SLV_TIM_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        TIM_ITConfig(PULSE_GEN_SLV_TIM_PORT, PULSE_GEN_SLV_TIM_IT, ENABLE);
    }
}

bool PulseGenConfig(u32 freq, f32 duty)  // ��������
{
#if 1

    if ((PULSE_GEN_MST_TIM_PORT->CR1 & TIM_CR1_CEN) ||
        (PULSE_GEN_SLV_TIM_PORT->CR1 & TIM_CR1_CEN))
    {
        // ����ʧ��: ������������
        return false;
    }

#endif

    if (freq == 0)
    {
        // ����ʧ��: ����Ƶ�ʲ���Ϊ��
        return false;
    }

    u16 prd, psc, ccr;

    if (TIM_Calc(PULSE_GEN_MST_TIM_FRQ, freq, freq * 0.02, &prd, &psc, nullptr) == false)
    {
        // ����ʧ��: �޷�����Ŀ��Ƶ�ʵ�����
        return false;
    }

    ccr = (prd + 1) * duty;

    if (ccr == 0)
    {
        // ����ʧ��: ռ�ձȲ���ȷ
        return false;
    }

    // �Ƚ�ֵ
    ccr -= 1;

    // Ƶ��
    PULSE_GEN_MST_TIM_PORT->PSC = psc;  // ��Ƶ
    PULSE_GEN_MST_TIM_PORT->ARR = prd;  // ����

    // ռ�ձ�

#if PULSE_GEN_MST_TIM_CH == 1
    PULSE_GEN_MST_TIM_PORT->CCR1 = ccr;
#elif PULSE_GEN_MST_TIM_CH == 4
    PULSE_GEN_MST_TIM_PORT->CCR4 = ccr;
#endif

    // ���óɹ�
    return true;
}

bool PulseGenStart(u32 count)  // ��������
{
    if ((PULSE_GEN_MST_TIM_PORT->CR1 & TIM_CR1_CEN) ||
        (PULSE_GEN_SLV_TIM_PORT->CR1 & TIM_CR1_CEN))
    {
        // ����ʧ��: ������������
        return false;
    }

    if (count == 0 || count > 65536)
    {
        // ����ʧ��: ȡֵ��Χ������������
        // TODO: 16Ϊλ��ʱ��,���ⵥ��������ʱ 65535 ���� 65536
        return false;
    }

    --count;

    if (count == 0)
    {
        // ������ģʽ
        TIM_SelectOnePulseMode(PULSE_GEN_MST_TIM_PORT, TIM_OPMode_Single);

        // enable pulse generator
        TIM_Cmd(PULSE_GEN_MST_TIM_PORT, ENABLE);
    }
    else
    {
        // PWM ģʽ
        TIM_SelectOnePulseMode(PULSE_GEN_MST_TIM_PORT, TIM_OPMode_Repetitive);

        // �������ɸ��� (�������2)
        TIM_SetAutoreload(PULSE_GEN_SLV_TIM_PORT, count);

        // enable pulse generator
        TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, ENABLE);
        TIM_Cmd(PULSE_GEN_MST_TIM_PORT, ENABLE);
        TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, ENABLE);
    }

    return true;
}

void PulseGenStop(void)
{
    // disable pulse generator
    TIM_Cmd(PULSE_GEN_MST_TIM_PORT, DISABLE);
    TIM_Cmd(PULSE_GEN_SLV_TIM_PORT, DISABLE);
    TIM_CtrlPWMOutputs(PULSE_GEN_MST_TIM_PORT, DISABLE);
}
