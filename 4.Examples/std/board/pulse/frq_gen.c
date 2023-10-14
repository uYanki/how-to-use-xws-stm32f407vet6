#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define clkin 168e6

int main()
{
    /*定义变量*/
    double            Total_time, Total_timeMax = 0.0;
    clock_t           start, finish;
    double            Error, ErrorMax = 0.0;  // 误差
    unsigned long int num = 0, num_temp = 0, ErrornumMax, Total_timenumMax;
    unsigned char     flag = 0, Symbol_Opt, Number_Offset;
    unsigned int      i, f, ErrorfMax, Total_timefMax;

    int xx = 0;

    unsigned short Psc;    // 对应周期下的预分配系数
    unsigned short Arr;    // 对应周期下的重装载值
    unsigned int   Cycle;  // STM32所有周期保存地址

    FILE* fp;  // 建立一个文件操作指针
    int   err;

    if ((err = fopen_s(&fp, "1.h", "w")) != 0)  // 以追加的方式建立或打开1.txt，默认位置在你程序的目录下面
    {
        printf("无法打开此文件\n");  // 如果打不开，就输出打不开
        exit(0);                     // 终止程序
    }

    for (f = 1; f < 2563; f++)  // j是频率：1 ~ 100k（100KHz）
    {
        // start = clock();
        num           = clkin / f;  // 100K是720
        num_temp      = num;
        Symbol_Opt    = 0;
        Number_Offset = 1;
        while (1)
        {
            flag = 0;
            for (i = 1; i < 65536; ++i)
            {
                if (((num_temp / i) < 65536) && (num_temp % i == 0))  // 满足条件说明有可以乘积的数值
                {                                                     // 对其进行保存
                    Psc   = i;
                    Arr   = num_temp / i;
                    Cycle = num_temp;
                    flag  = 1;
                    break;
                }
            }
            if (flag == 0)  // 说明这个数没有了，该对num进行处理了
            {
                if ((num <= (4294836225 - Number_Offset)) && (Symbol_Opt == 0))  // 65535 * 65535 = 4294836225
                {
                    num_temp   = num + Number_Offset;
                    Symbol_Opt = 1;
                }
                else if ((num - Number_Offset > 0) && (Symbol_Opt == 1))
                {
                    num_temp   = num - Number_Offset;
                    Symbol_Opt = 0;
                    Number_Offset++;
                }
            }
            else
            {
                break;
            }
        }

        /*输出到文件当中*/
        fprintf(fp, "0x%X,0x%X,", Psc, Arr);  // 同输出printf一样，以格式方式输出到文本中
        xx++;
        if (xx == 10)
        {
            xx = 0;
            fprintf(fp, "\\\n");  // 同输出printf一样，以格式方式输出到文本中
        }

        /*记录计算机计算时间*/
        // finish = clock();
        // Total_time = (double)(finish - start) / CLOCKS_PER_SEC; //单位换算成秒
        // if (Total_time > Total_timeMax)
        //{
        //	Total_timefMax = f;
        //	Total_timeMax = Total_time;
        //	Total_timenumMax = num;
        // }

        Error = (((float)clkin / (Psc * Arr)) - f) / f;  // 误差=（实际频率-理论频率）/理论频率。
        // if (Error > ErrorMax)
        //{
        //	ErrorfMax = f;
        //	ErrorMax = Error;
        //	ErrornumMax = num;
        // }
        // printf("f=%-8lu    num=%-8lu    %8lu = %5lu * %5lu",f, num, Cycle, Psc, Arr);
        // printf("    误差：%f%%\n", Error*100.0);
    }
    fclose(fp);  // 关闭流
    // printf("当mun为%8lu时，%8luHz,最长时间：%f\n", Total_timenumMax, Total_timefMax, Total_timeMax);
    printf("当mun为%8lu时，频率f为%8luHz时,最大误差：%f%%\n", ErrornumMax, ErrorfMax, ErrorMax * 100.0);
    /*
    当mun为 6545454时，最长时间：0.001000
    当mun为     721时，频率f为   99723Hz时,最大误差：0.138688%
    */
}