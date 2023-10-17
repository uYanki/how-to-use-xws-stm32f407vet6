#ifndef __PARA_TABLE_H__
#define __PARA_TABLE_H__

#include "attr.h"

//-----------------------------------------------------------------------------
//

#define ADDR_FMT_DEC 1
#define ADDR_FMT_HEX 2

#ifndef ADDR_FMT
#define ADDR_FMT ADDR_FMT_DEC
#endif

#if ADDR_FMT == ADDR_FMT_DEC
#define GROUP_SIZE 100
#elif ADDR_FMT == ADDR_FMT_HEX
#define GROUP_SIZE 255
#endif

//-----------------------------------------------------------------------------
//

// windows
// #pragma pack(2)
// #pragma pack()

#define EXPORT_PARA_GROUP

EXPORT_PARA_GROUP typedef __packed struct {
    u32 u32DrvScheme;       ///< P00.000 RO 驱动器方案
    u16 u16McuSwVerMajor;   ///< P00.002 RO 软件基线版本号
    u16 u16McuSwVerMinor;   ///< P00.003 RO 软件分支版本号
    u32 u32McuSwDate;       ///< P00.004 RO 软件日期
    u32 u32McuSwBuildDate;  ///< P00.006 RO 软件构建日期
    u16 u16MotType;         ///< P00.008 RW 电机类型
    u16 u16MotPolePairs;    ///< P00.009 RW 电机极对数
    u16 u16EncScheme;       ///< P00.010 RW 编码器方案
    u16 u16EncType;         ///< P00.011 RW 编码器类型
    u32 u32EncRes;          ///< P00.012 RW 编码器分辨率
    u32 u32EncOffset;       ///< P00.014 RW 编码器安装偏置
    u16 u16HallOffset;      ///< P00.016 RW 霍尔安装偏置
    u16 u16EncOverTemp;     ///< P00.017 RW 编码器过热点
    u16 u16CurRate;         ///< P00.018 RW 额定电流
    u16 u16CurMax;          ///< P00.019 RW 最大电流
    u16 u16TrqRate;         ///< P00.020 RW 额定转矩
    u16 u16TrqMax;          ///< P00.021 RW 最大转矩
    u16 u16SpdRate;         ///< P00.022 RW 额定转速
    u16 u16SpdMax;          ///< P00.023 RW 最大转速
    u16 u16BlinkPeriod;     ///< P00.024 RW 指示灯闪烁周期

} DrvCfg_t;

// u16 u16VoltInRate;  // 额定输入电压
// u16 u16VdMax;       // 最大Vd
// u16 u16VqMax;       // 最大Vq
// u16 u16PowerRate;   // 额定功率

// u16 u16AccMax;      // 最大加速度
// u16 u32InertiaWL;   // 转动惯量
// u16 u32InertiaWH;   // 转动惯量
// u16 u16StatorRes;   // 定子电阻
// u16 u16StatorLd;    // 定子D轴电感
// u16 u16StatorLq;    // 定子Q轴电感
// u16 u16EmfCoeff;    // 反电动势常数
// u16 u16TrqCoeff;    // 转矩系数
// u16 u16Tm;          // 机械时间常数
// u16 u16MotMidOlt;   // 连续过载时间
// u16 u16MotMaxOlt;   // 瞬时过载时间

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16Umdc;        ///< P01.000 RO 母线电压
    s16 s16CurPhAFb;    ///< P01.001 RO
    s16 s16CurPhBFb;    ///< P01.002 RO
    s16 s16IalphaFb;    ///< P01.003 RO
    s16 s16IbetaFb;     ///< P01.004 RO
    s16 s16IqFb;        ///< P01.005 RO
    s16 s16IdFb;        ///< P01.006 RO
    u16 u16ElecAng;     ///< P01.007 RO
    s16 s16Ualpha;      ///< P01.008 RO
    s16 s16Ubeta;       ///< P01.009 RO
    u16 u16DutyPha;     ///< P01.010 RO A相输出占空比
    u16 u16DutyPhb;     ///< P01.011 RO B相输出占空比
    u16 u16DutyPhc;     ///< P01.012 RO C相输出占空比
    s16 s16Uq;          ///< P01.013 RO
    s16 s16Ud;          ///< P01.014 RO
    s16 s16UserSpdRef;  ///< P01.015 RO 用户速度指令, 单位 rpm
    s16 s16UserSpdFb;   ///< P01.016 RO 用户速度反馈, 单位 rpm
    s64 s64UserPosRef;  ///< P01.017 RO 用户位置指令
    s64 s64UserPosFb;   ///< P01.021 RO 用户位置反馈
    s16 s16UserTrqRef;  ///< P01.025 RO 用户转矩指令
    s16 s16UserTrqFb;   ///< P01.026 RO 用户转矩反馈

    s16 s16EnvTemp;  ///< P01.027 RO 环境温度(ds18b20), 单位 0.01°

} MotSta_t;

//-----------------------------------------------------------------------------
//

typedef enum {
    SPD_TGT_SRC_DIGITAL,
    SPD_TGT_SRC_MUTIL_DIGITAL,
    SPD_TGT_SRC_ANALOG,
} SpdTgtSrc_e;

typedef enum {
    SPD_LIM_FWD,
    SPD_LIM_REV,
} SpdLimSrc_e;

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16SpdTgtSrc;       ///< P02.000 RW 速度指令来源
    s16 s16SpdTgtRef;       ///< P02.001 RW 目标速度指令
    u16 u16SpdTgtUnit;      ///< P02.002 RW 速度指令单位
    u16 u16SpdPlanMode;     ///< P02.003 RW 速度规划模式
    u16 u16DecTime;         ///< P02.004 RW 减速时间
    u16 u16AccTime;         ///< P02.005 RW 加速时间
    s16 s16SpdDigRef;       ///< P02.006 RW 数字速度指令
    u16 u16SpdMulRefSel;    ///< P02.007 RW 多段指令选择
    s16 s16SpdDigRefs[16];  ///< P02.008 RW 数字速度指令
    u16 u16SpdAiCoeff;      ///< P02.024 RW 单位模拟量对应转速指令
    u16 u16SpdLimSrc;       ///< P02.025 RW 速度限制来源
    u16 u16SpdLimRes;       ///< P02.026 RW 速度限制状态
    u16 s64SpdFwdLim;       ///< P02.027 RW 正向速度限制
    u16 s64SpdRevLim;       ///< P02.028 RW 反向速度限制
    u16 u16SpdCoeff;        ///< P02.029 RW 速度缩放系数
} SpdCtl_t;

//-----------------------------------------------------------------------------
//

typedef enum {
    POS_TGT_SRC_DIGITAL,
    POS_TGT_SRC_MUTIL_DIGITAL,
} PosTgtSrc_e;

typedef enum {
    POS_LIM_FWD,
    POS_LIM_REV,
} PosLimSrc_e;

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16PosTgtSrc;       ///< P03.000 RW 位置指令来源
    s64 s16PosTgtRef;       ///< P03.001 RW 目标位置指令
    s64 s64PosDigRef;       ///< P03.005 RW 数字位置指令
    u16 u16SpdMulRefSel;    ///< P03.009 RW 多段指令选择
    s64 s64PosDigRefs[16];  ///< P03.010 RW 数字位置指令
    u16 u16PosLimSrc;       ///< P03.074 RW 位置限制来源
    u16 u16PosLimRes;       ///< P03.075 RO 位置限制状态
    u16 s64PosFwdLim;       ///< P03.076 RW 正向位置限制
    u16 s64PosRevLim;       ///< P03.077 RW 反向位置限制

    s64 s64MechOffset;    ///< P03.078 RW 机械位置偏置
    u32 u32ElecGearNum;   ///< P03.082 RW 电子齿轮比分子(numerator)
    u32 u32ElecGearDeno;  ///< P03.084 RW 电子齿轮比分母(denominator)

    u16 u16EncState;    ///< P03.086 RO 编码器状态
    s16 s16EncTemp;     ///< P03.087 RO 编码器温度
    u16 u16EncMode;     ///< P03.088 RW 编码器工作模式
    s64 s64EncPosInit;  ///< P03.089 RW 编码器上电初始位置
    s32 s32EncPos;      ///< P03.093 RO 编码器单圈位置
    s32 s32EncTurns;    ///< P03.095 RO 编码器圈数

    u16 u16EncFreqDivDir;   ///< P03.097 RW 编码器分频输出脉冲方向
    u16 u16EncFreqDivNum;   ///< P03.098 RW 编码器分频输出分子
    u16 u16EncFreqDivDeno;  ///< P03.099 RW 编码器分频输出分母

} PosCtl_t;

//-----------------------------------------------------------------------------
//

typedef __packed struct {
    u16 u16U;
    u16 u16Bais;
} AiCtrl_t;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16CommSlaveAddr;      ///< P04.000 RW 通讯从站地址
    u16 u16ModBaudrate;        ///< P04.001 RW Modbus 波特率
    u16 u16ModDataFmt;         ///< P04.002 RW Modbus 数据格式
    u16 u16ModMasterEndian;    ///< P04.003 RW Modbus 主站大小端
    u16 u16ModDisconnectTime;  ///< P04.004 RW Modbus 通讯断开检测时间
    u16 u16ModAckDelay;        ///< P04.005 RW Modbus 命令响应延时
    u16 u16CopBitrate;         ///< P04.006 RW CANopen 比特率
} SlvCom_t;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16LogSampPrd;   ///< P05.000 RW 数据记录采样周期设定
    u16 u16LogSampPts;   ///< P05.001 RW 数据记录采样点数设定
    u16 u16LogChCnt;     ///< P05.002 RW 数据记录通道数量设定
    u16 u16LogChSrc[8];  ///< P05.003 RW 数据记录通道地址设定 (数据源)
} CurveTrace_t;

//-----------------------------------------------------------------------------
//

/**
 * @brief 波形发生器 (DAC+DMA)
 *
 *   u32WaveTargetFreq(RW): 目标输出波形频率.
 *   u32WaveActualFreq(R): 实际输出波形频率.
 *      - 注意: 由于硬件性能限制导致大部分高频率段输出有误
 *   u16WaveFreqUnit(RW):
 *      - 单位: 毫赫 mHz, 赫兹 Hz ( 1e3 mHz = 1 Hz)
 *
 *   u16WaveType(RW): 波形类型
 *      * 预设波形:
 *         - @ WaveType_Noise:
 *              u16WaveAmplitude(RW): 噪声幅度
 *         - @ WaveType_Triangle:
 *              u16WaveAmplitude(RW): 三角波振幅
 *      * 自定义波形:
 *         - @ WaveType_Custom: 自定义波形
 *              u16WaveAlign(RW):  数据对齐方式  |     12bit       |      8bit
 *              u16WaveSizea(RW):  数据点数量    |    最多80点     |    最多160点
 *              u16WaveDataa(RW):  数据点数值    |  每个数值为1点   |  每个数值为2点
 *
 *   u16WaveConfig(RW): 写0触发波形配置, 配置成功自动置为1, 配置失败自动置为2
 */

typedef enum {
    WaveConfig_Doit    = 0,  // 触发波形配置
    WaveConfig_Success = 1,  // 波形配置成功
    WaveConfig_Failure = 2,  // 波形配置失败
    _WaveConfig_Count  = 3,
} WaveConfig_e;

typedef enum {
    WaveFreqUnit_Hz     = 0,
    WaveFreqUnit_mHz    = 1,
    _WaveFreqUnit_Count = 2,
} WaveFreqUnit_e;

typedef enum {
    WaveType_None     = 0,  // 无
    WaveType_Noise    = 1,  // 噪声
    WaveType_Triangle = 2,  // 三角波
    WaveType_Sine     = 3,  // 正弦波
    WaveType_Custom   = 4,  // 自定义
    _WaveType_Count   = 5,
} WaveType_e;

typedef enum {
    WaveAlign_12Bit_Right = 0,  // 12位右对齐
    WaveAlign_12Bit_Left  = 1,  // 12位左对齐
    WaveAlign_8Bit_Right  = 2,  //  8位右对齐
    _WaveAlign_Count      = 3,
} WaveAlign_e;

// 电压范围 ( 0~4095 -> 0~3.3v )
typedef enum {
    WaveAmplitude_1      = 0x0,  // 0 ~ 1
    WaveAmplitude_3      = 0x1,  // 0 ~ 3
    WaveAmplitude_7      = 0x2,  // 0 ~ 7
    WaveAmplitude_15     = 0x3,  // 0 ~ 15
    WaveAmplitude_31     = 0x4,  // 0 ~ 31
    WaveAmplitude_63     = 0x5,  // 0 ~ 63
    WaveAmplitude_127    = 0x6,  // 0 ~ 127
    WaveAmplitude_255    = 0x7,  // 0 ~ 255
    WaveAmplitude_511    = 0x8,  // 0 ~ 511
    WaveAmplitude_1023   = 0x9,  // 0 ~ 1023
    WaveAmplitude_2047   = 0xA,  // 0 ~ 2047
    WaveAmplitude_4095   = 0xB,  // 0 ~ 4095
    _WaveAmplitude_Count = 12,
} WaveAmplitude_e;

// 波形同步输出(共用时钟源)
typedef enum {
    WaveSync_Disable,  // 同步输出
    WaveSync_Enable,   // 异步输出
} WaveSync_e;

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16WaveConfig;      ///< P06.000 RW 触发配置
    u16 u16WaveType;        ///< P06.001 RW 波形类型
    u32 u32WaveTargetFreq;  ///< P06.002 RW 波形期望频率
    u32 u32WaveActualFreq;  ///< P06.004 RO 波形实际频率
    u16 u16WaveFreqUnit;    ///< P06.006 RW 波形频率单位
    u16 u16WaveAmplitude;   ///< P06.007 RW 波形振幅
    u16 u16WaveAlign;       ///< P06.008 RW 数据点对齐方式
    u16 u16WaveSize;        ///< P06.009 RW 数据点数量
    u16 u16WaveData[64];    ///< P06.010 RW 数据点数值
} WaveGen_t;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef __packed struct {
    u16 u16DbgBuf[8];  ///< P07.000
    u32 u32DbgBuf[8];  ///< P07.008
    u64 u64DbgBuf[8];  ///< P07.024
    f32 f32DbgBuf[8];  ///< P07.056
    f64 f64DbgBuf[8];  ///< P07.072
} DbgSta_t;

//-----------------------------------------------------------------------------
//

typedef struct {
    // clang-format off

    union {
        u16 RESV0[GROUP_SIZE];
        DrvCfg_t DrvCfg;
    };
    
    union {
        u16 RESV1[GROUP_SIZE];
        MotSta_t MotSta;
    };
    
    union {
        u16 RESV2[GROUP_SIZE];
        SpdCtl_t SpdCtl;
    };
    
    union {
        u16 RESV3[GROUP_SIZE];
        PosCtl_t PosCtl;
    };

    union {
        u16 RESV4[GROUP_SIZE];
        WaveGen_t WaveGen1;
    };

    union {
        u16 RESV5[GROUP_SIZE];
        WaveGen_t WaveGen2;
    };

    union {
        struct {
            u16 RESV6[GROUP_SIZE];
            u16 RESV7[GROUP_SIZE];
        };
        DbgSta_t DbgSta;
    };

    // clang-format on
} ParaTable_t;

//-----------------------------------------------------------------------------
//

extern ParaTable_t m_paratbl;

#define P(grp)      m_paratbl.grp
#define P_ADDR(grp) &P(grp)

#endif
