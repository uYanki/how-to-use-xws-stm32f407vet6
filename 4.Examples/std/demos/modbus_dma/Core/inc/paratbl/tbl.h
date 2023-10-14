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

typedef __packed struct {
    u32 u32DrvScheme;       ///< 000 驱动器方案
    u16 u16McuSwVerMajor;   ///< 002 软件基线版本号
    u16 u16McuSwVerMinor;   ///< 003 软件分支版本号
    u32 u32McuSwDate;       ///< 004 软件日期
    u32 u32McuSwBuildDate;  ///< 006 软件构建日期
    u16 u16MotType;         ///< 008 电机类型
    u16 u16MotPolePairs;    ///< 009 电机极对数
    u16 u16EncScheme;       ///< 010 编码器方案
    u16 u16EncType;         ///< 011 编码器类型
    u32 u32EncRes;          ///< 012 编码器分辨率
    u32 u32EncOffset;       ///< 014 编码器安装偏置
    u16 u16HallOffset;      ///< 016 霍尔安装偏置
    u16 u16EncOverTemp;     ///< 017 编码器过热点
    u16 u16CurRate;         ///< 018 额定电流
    u16 u16CurMax;          ///< 019 最大电流
    u16 u16TrqRate;         ///< 020 额定转矩
    u16 u16TrqMax;          ///< 021 最大转矩
    u16 u16SpdRate;         ///< 022 额定转速
    u16 u16SpdMax;          ///< 023 最大转速
    u16 u16BlinkPeriod;     ///< 024 指示灯闪烁周期
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

typedef __packed struct {
    u16 u16Umdc;        ///< 000 母线电压
    s16 s16CurPhAFb;    ///< 001 
    s16 s16CurPhBFb;    ///< 002 
    s16 s16IalphaFb;    ///< 003 
    s16 s16IbetaFb;     ///< 004 
    s16 s16IqFb;        ///< 005 
    s16 s16IdFb;        ///< 006 
    u16 u16ElecAng;     ///< 007 
    s16 s16Ualpha;      ///< 008 
    s16 s16Ubeta;       ///< 009 
    u16 u16DutyPha;     ///< 010 A相输出占空比
    u16 u16DutyPhb;     ///< 011 B相输出占空比
    u16 u16DutyPhc;     ///< 012 C相输出占空比
    s16 s16Uq;          ///< 013 
    s16 s16Ud;          ///< 014 
    s16 s16UserSpdRef;  ///< 015 用户速度指令
    s16 s16UserSpdFb;   ///< 016 用户速度反馈
    s64 s64UserPosRef;  ///< 017 用户位置指令
    s64 s64UserPosFb;   ///< 021 用户位置反馈
    s16 s16UserTrqRef;  ///< 025 
    s16 s16UserTrqFb;   ///< 026 用户转矩反馈

    s16 s16EnvTemp;  ///< 027 环境温度(ds18b20), 单位 0.01°

    u64 u64DbgBuf[4];   ///< 028 
    u32 u32DbgBuf[8];   ///< 044 
    u16 u16DbgBuf[16];  ///< 060 
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

typedef __packed struct {
    u16 u16SpdTgtSrc;       ///< 000 速度指令来源
    s16 s16SpdTgtRef;       ///< 001 目标速度指令
    u16 u16SpdTgtUnit;      ///< 002 速度指令单位
    u16 u16SpdPlanMode;     ///< 003 速度规划模式
    u16 u16DecTime;         ///< 004 减速时间
    u16 u16AccTime;         ///< 005 加速时间
    s16 s16SpdDigRef;       ///< 006 数字速度指令
    u16 u16SpdMulRefSel;    ///< 007 多段指令选择
    s16 s16SpdDigRefs[16];  ///< 008 数字速度指令
    u16 u16SpdAiCoeff;      ///< 024 单位模拟量对应转速指令
    u16 u16SpdLimSrc;       ///< 025 速度限制来源
    u16 u16SpdLimRes;       ///< 026 速度限制状态
    u16 s64SpdFwdLim;       ///< 027 正向速度限制
    u16 s64SpdRevLim;       ///< 028 反向速度限制
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

typedef __packed struct {
    u16 u16PosTgtSrc;       ///< 000 位置指令来源
    s64 s16PosTgtRef;       ///< 001 目标位置指令
    s64 s64PosDigRef;       ///< 005 数字位置指令
    u16 u16SpdMulRefSel;    ///< 007 多段指令选择
    s64 s64PosDigRefs[16];  ///< 010 数字位置指令
    u16 u16PosLimSrc;       ///< 074 位置限制来源
    u16 u16PosLimRes;       ///< 075 位置限制状态
    u16 s64PosFwdLim;       ///< 076 正向位置限制
    u16 s64PosRevLim;       ///< 077 反向位置限制

    s64 s64MechOffset;    ///< 078 机械位置偏置
    u32 u32ElecGearNum;   ///< 082 电子齿轮比分子
    u32 u32ElecGearDeno;  ///< 084 电子齿轮比分母

    u16 u16EncState;    ///< 086 编码器状态
    s16 s16EncTemp;     ///< 087 编码器温度
    u16 u16EncContErr;  ///< 088 编码器单次连续出错最大数
    u16 u16EncErrSum;   ///< 089 编码器出错累计数
    u16 u16EncErrType;  ///< 090 编码器错误类型
    u16 u16EncMode;     ///< 091 编码器工作模式
    s64 s64EncPosInit;  ///< 092 编码器上电初始位置
    s32 s32EncPos;      ///< 096 编码器单圈位置
    s32 s32EncTurns;    ///< 098 编码器圈数

    u16 u16EncFreqDivDir;   ///< 100 编码器分频输出脉冲方向
    u16 u16EncFreqDivNum;   ///< 101 编码器分频输出分子
    u16 u16EncFreqDivDeno;  ///< 102 编码器分频输出分母
} PosCtl_t;

//-----------------------------------------------------------------------------
//

typedef __packed struct {
    u16 u16U;
    u16 u16Bais;
} AiCtrl_t;

//-----------------------------------------------------------------------------
//

typedef __packed struct {
    u16 u16CommSlaveAddr;      ///< 000 RW 通讯从站地址
    u16 u16ModBaudrate;        ///< 001 RW Modbus 波特率
    u16 u16ModDataFmt;         ///< 002 RW Modbus 数据格式
    u16 u16ModMasterEndian;    ///< 003 RW Modbus 主站大小端
    u16 u16ModDisconnectTime;  ///< 004 RW Modbus 通讯断开检测时间
    u16 u16ModAckDelay;        ///< 005 RW Modbus 命令响应延时
    u16 u16CopBitrate;         ///< 006 RW CANopen 比特率
} SlvCom_t;

//-----------------------------------------------------------------------------
//

typedef __packed struct {
    u16 u16LogSampPrd;   ///< 000 RW 数据记录采样周期设定
    u16 u16LogSampPts;   ///< 001 RW 数据记录采样点数设定
    u16 u16LogChCnt;     ///< 002 RW 数据记录通道数量设定
    u16 u16LogChSrc[8];  ///< 003 RW 数据记录通道地址设定 (数据源)
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

typedef __packed struct {
    u16 u16WaveConfig;      ///< 000 RW 触发配置
    u16 u16WaveType;        ///< 001 RW 波形类型
    u32 u32WaveTargetFreq;  ///< 002 RW 波形期望频率
    u32 u32WaveActualFreq;  ///< 004 RO 波形实际频率
    u16 u16WaveFreqUnit;    ///< 006 RW 波形频率单位
    u16 u16WaveAmplitude;   ///< 007 RW 波形振幅
    u16 u16WaveAlign;       ///< 008 RW 数据点对齐方式
    u16 u16WaveSize;        ///< 009 RW 数据点数量
    u16 u16WaveData[64];    ///< 010 RW 数据点数值
} WaveGen_t;

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
        struct {
            u16 RESV3[GROUP_SIZE];
            u16 RESV4[GROUP_SIZE];
        };
        PosCtl_t PosCtl;
    };

    union {
        u16 RESV5[GROUP_SIZE];
        WaveGen_t WaveGen1;
    };

    union {
        u16 RESV6[GROUP_SIZE];
        WaveGen_t WaveGen2;
    };

    // clang-format on
} ParaTable_t;

//-----------------------------------------------------------------------------
//

extern ParaTable_t m_paratbl;

#define P(group) m_paratbl.group

#endif
