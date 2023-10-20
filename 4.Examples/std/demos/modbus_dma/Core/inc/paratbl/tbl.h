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

// 旋转正方向
typedef enum {
    ROT_FWD_DIR_CCW = 0,  ///< 逆时针为正转（编码器计数增加）
    ROT_FWD_DIR_CW,       ///< 顺时针为正转
} RotFwdDir_e;

// 旋转方向
typedef enum {
    ROT_DIR_FWD = 0,  ///< 正转
    ROT_DIR_REV,      ///< 反转
} RotDir_e;

// windows
// #pragma pack(2)
// #pragma pack()

#define EXPORT_PARA_GROUP

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u32 u32DrvScheme;       ///< P00.000 RO 驱动器方案
        u16 u16McuSwVerMajor;   ///< P00.002 RO 软件基线版本号
        u16 u16McuSwVerMinor;   ///< P00.003 RO 软件分支版本号
        u16 u16McuSwVerPatch;   ///< P00.004 RO 软件补丁版本号
        u32 u32McuSwBuildDate;  ///< P00.005 RO 软件构建日期
        u16 u16RunTimeLim;      ///< P00.007 RW 运行时间限制
        u16 u16ExtDiNum;        ///< P00.008 RO 外部DI端子个数
        u16 u16ExtDoNum;        ///< P00.009 RO 外部DO端子个数
        u16 u16ExtAiNum;        ///< P00.010 RO 外部AI端子个数
        u16 u16TempNum;         ///< P00.011 RO 温度采样个数
        u16 u16BlinkPeriod;     ///< P00.012 RW 指示灯闪烁周期
        u16 u16MotType;         ///< P00.013 RW 电机类型
        u16 u16MotPolePairs;    ///< P00.014 RW 电机极对数
        u16 u16EncScheme;       ///< P00.015 RW 编码器方案
        u16 u16EncType;         ///< P00.016 RW 编码器类型
        u32 u32EncRes;          ///< P00.017 RW 编码器分辨率
        u32 u32EncOffset;       ///< P00.019 RW 编码器安装偏置
        u16 u16HallOffset;      ///< P00.021 RW 霍尔安装偏置(电角度偏置)
        u16 u16EncOverTemp;     ///< P00.022 RW 编码器过热点
        u16 u16CurRate;         ///< P00.023 RW 额定电流
        u16 u16CurMax;          ///< P00.024 RW 最大电流
        u16 u16TrqRate;         ///< P00.025 RW 额定转矩
        u16 u16TrqMax;          ///< P00.026 RW 最大转矩
        u16 u16SpdRate;         ///< P00.027 RW 额定转速
        u16 u16SpdMax;          ///< P00.028 RW 最大转速
    };
    u16 GROUP[GROUP_SIZE];  ///< P00.028
} DrvCfg_u;

//-----------------------------------------------------------------------------
//

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

typedef struct {
    s32 s32MotEncRes;     ///< 编码器精度(一圈脉冲数)
    s32 s32MotEncOffset;  ///< 编码器安装偏置
    u16 u16MotPolePair;   ///< motor pole pair
} LogicFunc_Str;

typedef struct
{
    u16 u16ServoOn;        ///< P07.000 伺服使能/PWM使能
    u16 u16FocCfg;         ///< P07.001 FOC模式配置
    s16 s16ElecAngOffset;  ///< P07.002 电角度补偿值
    u16 u16PolePair;       ///< P07.003 电机极对数
    u16 u16AbsResBit;      ///< P07.004 绝对值编码器分辨率位宽
    u16 u16IncResLine;     ///< P07.005 增量式编码器单圈分辨率
    u16 u16FocStatus;      ///< P07.006 FOC状态
    s16 s16CurOffsetU;     ///< P07.010 U相电流零偏
    s16 s16CurOffsetV;     ///< P07.011 V相电流零偏
    u16 u16TdCnt;          ///< P07.031 PWM死区计数值设置
    s16 s16CurPhAFb;       ///< P07.032 A相反馈电流
    s16 s16CurPhBFb;       ///< P07.033 B相反馈电流
    s16 s16IalphaFb;       ///< P07.034 Alpha轴反馈电流
    s16 s16IbetaFb;        ///< P07.035 Beta轴反馈电流
    s16 s16IqFb;           ///< P07.036 Q轴反馈电流
    s16 s16IdFb;           ///< P07.037 D轴反馈电流
    s16 s16VqRef;          ///< P07.038 Q轴电压指令（MCU设定）
    s16 s16VdRef;          ///< P07.039 D轴电压指令（MCU设定）
    s16 s16ValphaRef;      ///< P07.040 Alpha轴电压指令（MCU设定）
    s16 s16VbetaRef;       ///< P07.041 Beta轴电压指令（MCU设定）
    u16 u16SvpwmTa;        ///< P07.042 A相占空比指令（MCU设定）
    u16 u16SvpwmTb;        ///< P07.043 B相占空比指令（MCU设定）
    u16 u16SvpwmTc;        ///< P07.044 C相占空比指令（MCU设定）
    u16 u16SvpwmSector;    ///< P07.045 输出电压矢量扇区
    u16 u16ElecAngRef;     ///< P07.046 电角度指令（MCU设定）
    u16 u16arRsvd4;        ///< P07.047 保留

    u16 u16arRsvd5;        ///< P07.054 保留
    u16 u16PosLoopStatus;  ///< P07.055 位置环相位标志
    s16 s16Uq;             ///< P07.056 Q轴输出电压（实际值）
    s16 s16Ud;             ///< P07.057 D轴输出电压（实际值）
    u16 u16arRsvd6[6];     ///< P07.058 ~ gp07.063 保留

} LogicParaGp07_Str;

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u32 u32SingleRotPos;    ///< P01.000 RO 单圈位置
        u32 u32MultiRotPos;     ///< P01.002 RO 多圈位置
        u16 u16PulseCntPerT;    ///< P01.004 增量式单周期编码器脉冲数
        u16 u16Umdc;            ///< P01.005 RO 母线电压
        u16 u16CurPhaseU;       ///< P01.006 RO U相电流
        u16 u16CurPhaseV;       ///< P01.007 RO V相电流
        u16 u16UmdcSamp;        ///< P01.008 RO 主回路母线电压
        s16 s16CurPhAFb;        ///< P01.009 RO
        s16 s16CurPhBFb;        ///< P01.010 RO
        s16 s16IalphaFb;        ///< P01.011 RO
        s16 s16IbetaFb;         ///< P01.012 RO
        s16 s16IqFb;            ///< P01.013 RO Q轴反馈电流
        s16 s16IdFb;            ///< P01.014 RO D轴反馈电流
        s16 s16CurPhAFb;        ///< P01.015 A相反馈电流
        s16 s16CurPhBFb;        ///< P01.016 B相反馈电流
        s16 s16IalphaFb;        ///< P01.017 Alpha轴反馈电流
        s16 s16IbetaFb;         ///< P01.018 Beta轴反馈电流
        s16 s16IqFb;            ///< P01.019 Q轴反馈电流
        s16 s16IdFb;            ///< P01.020 D轴反馈电流
        u16 u16ElecAng;         ///< P01.021 RO 电角度(实际值)
        s16 s16Ualpha;          ///< P01.022 RO Alpha轴输出电压(实际值)
        s16 s16Ubeta;           ///< P01.023 RO Beta轴输出电压(实际值)
        u16 u16DutyPha;         ///< P01.024 RO A相输出占空比(实际值)
        u16 u16DutyPhb;         ///< P01.025 RO B相输出占空比(实际值)
        u16 u16DutyPhc;         ///< P01.026 RO C相输出占空比(实际值)
        s16 s16Uq;              ///< P01.027 RO
        s16 s16Ud;              ///< P01.028 RO
        s32 s32UserSpdRef;      ///< P01.029 RO 用户速度指令, 单位 rpm
        s32 s32UserSpdFb;       ///< P01.031 RO 用户速度反馈, 单位 rpm
        s64 s64UserPosRef;      ///< P01.033 RO 用户位置指令
        s64 s64UserPosFb;       ///< P01.037 RO 用户位置反馈
        s16 s16UserTrqRef;      ///< P01.041 RO 用户转矩指令
        s16 s16UserTrqFb;       ///< P01.042 RO 用户转矩反馈
        s16 s16EnvTemp;         ///< P01.043 RO 环境温度, 单位 0.01°C
        s16 s16ChipTemp;        ///< P01.044 RO 芯片温度, 单位 0.01°C
        u32 u32AdcSampTime;     ///< P01.045 RO ADC采样耗时
        u16 u16AdcSampRes[10];  ///< P01.047 RO ADC采样结果
    };
    u16 GROUP[GROUP_SIZE];  ///< P01.056
} DevSta_u;

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        s16 s16UdRef;  ///< P02.000 D轴电压指令
        s16 s16UqRef;  ///< P02.001 Q轴电压指令
    };
    u16 GROUP[GROUP_SIZE];  ///< P02.001
} MotDrv_u;

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

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16SpdTgtSrc;       ///< P03.000 RW 速度指令来源
        s16 s16SpdTgtRef;       ///< P03.001 RW 目标速度指令
        u16 u16SpdTgtUnit;      ///< P03.002 RW 速度指令单位
        u16 u16SpdPlanMode;     ///< P03.003 RW 速度规划模式
        u16 u16DecTime;         ///< P03.004 RW 减速时间
        u16 u16AccTime;         ///< P03.005 RW 加速时间
        s16 s16SpdDigRef;       ///< P03.006 RW 数字速度指令
        u16 u16SpdMulRefSel;    ///< P03.007 RW 多段指令选择
        s16 s16SpdDigRefs[16];  ///< P03.008 RW 数字速度指令
        u16 u16SpdAiCoeff;      ///< P03.024 RW 单位模拟量对应转速指令
        u16 u16SpdLimSrc;       ///< P03.025 RW 速度限制来源
        u16 u16SpdLimRes;       ///< P03.026 RW 速度限制状态
        u16 s64SpdFwdLim;       ///< P03.027 RW 正向速度限制
        u16 s64SpdRevLim;       ///< P03.028 RW 反向速度限制
        u16 u16SpdCoeff;        ///< P03.029 RW 速度缩放系数
    };
    u16 GROUP[GROUP_SIZE];  ///< P03.029
} SpdCtl_u;

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

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16PosTgtSrc;       ///< P04.000 RW 位置指令来源
        s64 s16PosTgtRef;       ///< P04.001 RW 目标位置指令
        s64 s64PosDigRef;       ///< P04.005 RW 数字位置指令
        u16 u16SpdMulRefSel;    ///< P04.009 RW 多段指令选择
        s64 s64PosDigRefs[16];  ///< P04.010 RW 数字位置指令
        u16 u16PosLimSrc;       ///< P04.074 RW 位置限制来源
        u16 u16PosLimRes;       ///< P04.075 RO 位置限制状态
        u16 s64PosFwdLim;       ///< P04.076 RW 正向位置限制
        u16 s64PosRevLim;       ///< P04.077 RW 反向位置限制
        s64 s64MechOffset;      ///< P04.078 RW 机械位置偏置
        u32 u32ElecGearNum;     ///< P04.082 RW 电子齿轮比分子 (numerator)
        u32 u32ElecGearDeno;    ///< P04.084 RW 电子齿轮比分母 (denominator)
        u16 u16EncState;        ///< P04.086 RO 编码器状态
        s16 s16EncTemp;         ///< P04.087 RO 编码器温度
        u16 u16EncMode;         ///< P04.088 RW 编码器工作模式
        s64 s64EncPosInit;      ///< P04.089 RW 编码器上电初始位置
        s32 s32EncPos;          ///< P04.093 RO 编码器单圈位置 (0,u32EncRes)
        s32 s32EncTurns;        ///< P04.095 RO 编码器圈数
        u16 u16EncFreqDivDir;   ///< P04.097 RW 编码器分频输出脉冲方向
        u16 u16EncFreqDivNum;   ///< P04.098 RW 编码器分频输出分子
        u16 u16EncFreqDivDeno;  ///< P04.099 RW 编码器分频输出分母
    };
    u16 GROUP[GROUP_SIZE];  ///< P04.099
} PosCtl_u;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16TrqRefSrc;       ///< P05.000 RW 转矩指令来源
        s16 s16TrqDigRef;       ///< P05.001 RW 数字转矩指令
        s16 s16TrqDigRefs[16];  ///< P05.002 RW 数字转矩指令
        u16 u16AiTrqCoeff;      ///< P05.018 RW 1V模拟量对应转矩指令
        u16 u16TrqLimFwd;       ///< P05.019 RW 正向转矩限制
        u16 u16TrqLimRev;       ///< P05.020 RW 反向限制
        u16 u16TrqLim;          ///< P05.021 RW 转矩限制状态
        u16 u16TrqModePosLim;   ///< P05.022 RW 转矩模式位置限制状态
        u16 u16TrqModeSpdLim;   ///< P05.023 RW 转矩模式速度限制状态
    };
    u16 GROUP[GROUP_SIZE];  ///< P05.023
} TrqCtl_u;

//-----------------------------------------------------------------------------
//

typedef __packed struct {
    u16 u16U;
    u16 u16Bais;
} AiCtrl_t;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16CommSlaveAddr;      ///< P06.000 RW 通讯从站地址
        u16 u16ModBaudrate;        ///< P06.001 RW Modbus 波特率
        u16 u16ModDataFmt;         ///< P06.002 RW Modbus 数据格式
        u16 u16ModMasterEndian;    ///< P06.003 RW Modbus 主站大小端
        u16 u16ModDisconnectTime;  ///< P06.004 RW Modbus 通讯断开检测时间
        u16 u16ModAckDelay;        ///< P06.005 RW Modbus 命令响应延时
        u16 u16CopBitrate;         ///< P06.006 RW CANopen 比特率
        u16 u16PdoInhTime;         ///< P06.007 RW CANopen PDO禁止时间
    };
    u16 GROUP[GROUP_SIZE];  ///< P06.007
} SlvCom_u;

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

// 波形同步输出(共用时钟源), [unsupport]
typedef enum {
    WaveSync_Disable,  // 同步输出
    WaveSync_Enable,   // 异步输出
} WaveSync_e;

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16WaveConfig;      ///< P07.000 RW 触发配置
        u16 u16WaveType;        ///< P07.001 RW 波形类型
        u32 u32WaveTargetFreq;  ///< P07.002 RW 波形期望频率
        u32 u32WaveActualFreq;  ///< P07.004 RO 波形实际频率
        u16 u16WaveFreqUnit;    ///< P07.006 RW 波形频率单位
        u16 u16WaveAmplitude;   ///< P07.007 RW 波形振幅
        u16 u16WaveAlign;       ///< P07.008 RW 数据点对齐方式
        u16 u16WaveSize;        ///< P07.009 RW 数据点数量
        u16 u16WaveData[80];    ///< P07.010 RW 数据点数值
    };
    u16 GROUP[GROUP_SIZE];  ///< P07.089
} WaveGen_u;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16DbgBuf[8];  ///< P08.000 RW
        u32 u32DbgBuf[8];  ///< P08.008 RW
        u64 u64DbgBuf[4];  ///< P08.024 RW
        f32 f32DbgBuf[8];  ///< P08.040 RW
    };
    u16 GROUP[GROUP_SIZE];  ///< P08.055
} DbgSta_u;

//-----------------------------------------------------------------------------
//

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u32 u32ErrMask;         ///< P09.000 RW 故障屏蔽字
        u32 u32WrnMask;         ///< P09.002 RW 警告屏蔽字
        u32 u32FaultMask;       ///< P09.004 RW 内部异常屏蔽字
        u16 u16AlmLog[16];      ///< P09.006 RO 报警历史
        u32 u32AlmLogTime[16];  ///< P09.022 RO 报警历史时间戳
    };
    u16 GROUP[GROUP_SIZE];  ///< P09.053
} Alarm_u;

//-----------------------------------------------------------------------------
// 曲线跟踪

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16LogMode;           ///< P10.000 RW 数据记录工作模式
        u16 u16LogAlmNoReq;       ///< P10.001 RW 数据记录故障记录号请求
        u16 u16LogTrigState;      ///< P10.002 RW 数据记录执行状态
        u16 u16LogTrigObj;        ///< P10.003 RW 数据记录触发对象
        s64 s64LogTrigThreshold;  ///< P10.004 RW 数据记录触发水平
        u16 u16LogTrigParaAddr;   ///< P10.008 RW 数据记录任意触发参数地址
        u16 u16LogTrigEdge;       ///< P10.009 RW 数据记录触发条件
        u16 u16LogTrigSet;        ///< P10.010 RW 数据记录触发设定
        u16 u16LogTrigIndex;      ///< P10.011 RW 数据记录触发成功数据索引
        u16 u16LogShakeHand;      ///< P10.012 RW 数据记录数据上传握手标志
        u16 u16LogUploadErr;      ///< P10.013 RW 数据记录数据上传失败原因
        u16 u16LogSampPrd;        ///< P10.014 RW 数据记录采样周期设定
        u16 u16LogSampPts;        ///< P10.015 RW 数据记录采样点数设定
        u16 u16LogChNbr;          ///< P10.016 RW 数据记录通道数量设定
        u16 u16LogChSrc[8];       ///< P10.017 RW 数据记录通道来源设定
    };
    u16 GROUP[GROUP_SIZE];  ///< P10.024
} CurveTrace_u;

//-----------------------------------------------------------------------------
// 多段运行模式

EXPORT_PARA_GROUP typedef union {
    __packed struct {
        u16 u16MultStepMode;       ///< P11.000 RW 多段运行模式选择
        u16 u16StartStep;          ///< P11.001 RW 起始段选择
        u16 u16EndStep;            ///< P11.002 RW 终点段选择
        u16 u16Resv0;              ///< P11.003 NO 保留
        s32 s32StepRefs[16];       ///< P11.004 RW 位置/速度指令
        u16 u16StepAccTimes[16];   ///< P11.036 RW 加速时间
        u16 u16StepDecTimes[16];   ///< P11.052 RW 减速时间
        u16 u16StepSpdLims[16];    ///< P11.068 RW 运行速度上限
        u16 u16StepHoldTimes[16];  ///< P11.084 RW 运行/等待时间
    };
    u16 GROUP[GROUP_SIZE];  ///< P11.099
} MulStp_u;

//-----------------------------------------------------------------------------
//

typedef struct {
    DrvCfg_u  DrvCfg;
    MotSta_u  MotSta;
    SpdCtl_u  SpdCtl;
    PosCtl_u  PosCtl;
    WaveGen_u WaveGen1;
    WaveGen_u WaveGen2;
    DbgSta_u  DbgSta;
} ParaTable_t;

//-----------------------------------------------------------------------------
//

extern ParaTable_t m_paratbl;

#define P(grp)      m_paratbl.grp
#define P_ADDR(grp) &P(grp)

#endif
