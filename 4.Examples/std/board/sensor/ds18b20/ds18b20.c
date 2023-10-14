#include "pinmap.h"

#include "system/sleep.h"

#include "ds18b20.h"

//-----------------------------------------------------------------------------
// port

#if 0

#define DQ_H()    GPIO_WriteBit(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN, Bit_SET)
#define DQ_L()    GPIO_WriteBit(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN, Bit_RESET)

#define DQ_GET()  GPIO_ReadInputDataBit(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN)
#define DQ_IS_L() (GPIO_ReadInputDataBit(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN) == Bit_RESET)
#define DQ_IS_H() (GPIO_ReadInputDataBit(DS18B20_DQ_GPIO_PORT, DS18B20_DQ_GPIO_PIN) == Bit_SET)

#else

#include "pinctrl.h"

#define DQ_H()    (PEout(DS18B20_DQ_GPIO_PINSRC) = 1)
#define DQ_L()    (PEout(DS18B20_DQ_GPIO_PINSRC) = 0)

#define DQ_GET()  (PEin(DS18B20_DQ_GPIO_PINSRC))
#define DQ_IS_L() (DQ_GET() == 0)
#define DQ_IS_H() (DQ_GET() == 1)

#endif

//

static GPIO_InitTypeDef GPIO_InitStruct = {
    .GPIO_Pin   = DS18B20_DQ_GPIO_PIN,
    .GPIO_Mode  = GPIO_Mode_OUT,
    .GPIO_Speed = GPIO_Speed_100MHz,
    .GPIO_OType = GPIO_OType_OD,
    .GPIO_PuPd  = GPIO_PuPd_UP,
};

void DQ_OUT(void)
{
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_Init(DS18B20_DQ_GPIO_PORT, &GPIO_InitStruct);
}

void DQ_IN(void)
{
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(DS18B20_DQ_GPIO_PORT, &GPIO_InitStruct);
}

//-----------------------------------------------------------------------------
//

void ds18b20_init(void)
{
    RCC_AHB1PeriphClockCmd(DS18B20_DQ_GPIO_CLK, ENABLE);
    GPIO_Init(DS18B20_DQ_GPIO_PORT, &GPIO_InitStruct);
}

// false:器件不存在 true:器件存在
bool ds18b20_reset(void)
{
    // reset

    DQ_OUT();

    DQ_L();
    sleep_us(520);  // 480~960us
    DQ_H();
    sleep_us(15);  // 15~60us

    // check

    u8 retry;
    DQ_IN();

    // wait for low level
    retry = 0;
    while (DQ_IS_H())
    {
        sleep_us(10);
        if (++retry > 20)
        {
            return false;
        }
    }

    // wait for high level
    retry = 0;
    while (DQ_IS_L())
    {
        sleep_us(10);
        if (++retry > 24)
        {
            return false;
        }
    }

    return true;
}

u8 ds18b20_read_bit(void)
{
    u8 dat;
    DQ_OUT();
    DQ_L();
    sleep_us(15);  // 拉低总线 ≥15us
    DQ_IN();
    dat = DQ_GET();
    sleep_us(40);  // 读取周期 60us
    return dat;
}

u8 ds18b20_read_byte(void)
{
    u8 data = 0x00, mask;
    for (mask = 0x01; mask != 0; mask <<= 1)
    {
        if (ds18b20_read_bit() == Bit_SET)
        {
            data |= mask;
        }
    }
    return data;
}

void ds18b20_write_byte(u8 data)
{
    DQ_OUT();
    for (u8 mask = 0x01; mask != 0; mask <<= 1)
    {
        if ((data & mask) == Bit_RESET)
        {
            // write 0
            DQ_L();
            sleep_us(70);  // 拉低总线 ≥60us
            DQ_H();
            sleep_us(2);  // 恢复时间, >1us
        }
        else
        {
            // write 1
            DQ_L();
            sleep_us(9);  // 拉低总线 1~15us
            DQ_H();
            sleep_us(55);  // 拉高总线 ≥60us
        }
    }
}

// 读温度寄存器的值（原始数据）
bool ds18b20_read_temp(s16* temp)
{
    u8 temp1, temp2;

    if (false == ds18b20_reset())
    {
        return 0;  // 总线复位
    }

    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0x44);  // 温度转换命令

    ds18b20_reset();  // 总线复位

    ds18b20_write_byte(0xcc);
    ds18b20_write_byte(0xbe);  // 读暂存器

    temp1 = ds18b20_read_byte();  // 读温度值低字节 LSB
    temp2 = ds18b20_read_byte();  // 读温度值高字节 MSB

    *temp = (temp2 << 8) | temp1;

    return true;
}

bool ds18b20_read_temp_by_id(u8 id[8], s16* temp)
{
    u8 temp1, temp2;
    u8 i;

    if (false == ds18b20_reset())
    {
        return false;
    }

    // Match Rom
    ds18b20_write_byte(0x55);
    for (i = 0; i < 8; i++)
    {
        ds18b20_write_byte(id[i]);
    }

    ds18b20_write_byte(0x44);  // 温度转换命令

    ds18b20_reset();

    ds18b20_write_byte(0x55);
    for (i = 0; i < 8; i++)
    {
        ds18b20_write_byte(id[i]);
    }

    ds18b20_write_byte(0xbe);

    temp1 = ds18b20_read_byte();  // 读温度值低字节 LSB
    temp2 = ds18b20_read_byte();  // 读温度值高字节 MSB

    *temp = (temp2 << 8) | temp1; /* 返回16位寄存器值 */

    return true;
}

f32 ds18b20_convert_temp(s16 temp)
{
    return ((temp < 0) ? (~temp + 1) : temp) * 0.0625;  // 0.0625*16=1
}

// 读DS18B20的ROM里的ID
// return: 0 表示失败， 1表示检测到正确ID
bool ds18b20_read_id(u8* id)
{
    u8 i;

    if (false == ds18b20_reset())
    {
        return false;
    }

    // ReadRom
    ds18b20_write_byte(0x33);

    for (i = 0; i < 8; i++)
    {
        id[i] = ds18b20_read_byte();
    }

    ds18b20_reset();
    return true;
}

bool ds18b20_set_resolution(u8 res /*9~12*/)
{
    if (false == ds18b20_reset())
    {
        return false;
    }

    // 精度设置
    switch (res)
    {
        case 9: res = 0; break;
        case 10: res = 1; break;
        case 11: res = 2; break;
        case 12: res = 3; break;
        default: return false;
    }

    // 写暂存器指令
    ds18b20_write_byte(0x4e);

    ds18b20_write_byte(0xff);  // TH
    ds18b20_write_byte(0xff);  // TL

    // 设置精度 0 res[1-0] 11111
    ds18b20_write_byte(0x1f | (res << 5));

    return true;
}

static u8 dallas_crc8(u8* data, u8 size)
{
    u8 crc = 0, inbyte, j, mix;

    uint32_t i;

    for (i = 0; i < size; ++i)
    {
        inbyte = data[i];
        for (j = 0; j < 8; ++j)
        {
            mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix)
            {
                crc ^= 0x8C;
            }
            inbyte >>= 1;
        }
    }

    return crc;
}

bool ds18b20_read_temp_crc(s16* temp)
{
    u8 i;
    u8 tmp[10], crc;

    for (i = 0; i < 3; i++)
    {
        if (ds18b20_reset() == 0)
        {
            continue;
        }

        ds18b20_write_byte(0xcc); /* 发命令 */
        ds18b20_write_byte(0x44); /* 发转换命令 */

        ds18b20_reset(); /* 总线复位 */

        ds18b20_write_byte(0xcc); /* 发命令 */
        ds18b20_write_byte(0xbe);

        tmp[0] = ds18b20_read_byte(); /* 读温度值低字节 */
        tmp[1] = ds18b20_read_byte(); /* 读温度值高字节 */

        tmp[2] = ds18b20_read_byte(); /* Alarm High Byte    */
        tmp[3] = ds18b20_read_byte(); /* Alarm Low Byte     */
        tmp[4] = ds18b20_read_byte(); /* Reserved Byte 1    */
        tmp[5] = ds18b20_read_byte(); /* Reserved Byte 2    */
        tmp[6] = ds18b20_read_byte(); /* Count Remain Byte  */
        tmp[7] = ds18b20_read_byte(); /* Count Per degree C */
        crc    = ds18b20_read_byte();

        /* 最高的125°对应0x07D0，这里返回0x0800表示错误*/
        if (crc != dallas_crc8(tmp, 8))
        {
            continue;
        }

        *temp = (tmp[1] << 8) | tmp[0]; /* 返回16位寄存器值 */

        return true; /* 正确 */
    }

    return false; /* 出错 */
}
