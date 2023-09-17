### arm_add_f32

（并行运算，与硬件的加法器个数有关）

168MHz，2 个元素个数为 1000 的浮点数组相加。

耗时：

| GRP_CNT | Time     |
| ------- | -------- |
| = 0     | 12028 us |
| = 1     | 9034 us  |
| = 2     | 8034 us  |
| = 3     | 7534 us  |
| >= 4    | 7300 us  |

```c
#define GRP_CNT 0u  // group count

void arm_add_f32(
    float*   pSrcA,
    float*   pSrcB,
    float*   pDst,
    uint32_t blockSize)
{
    uint32_t blkCnt;

    blkCnt = blockSize >> GRP_CNT;  // ÷

    while (blkCnt > 0)
    {
        *pDst++ = (*pSrcA++) + (*pSrcB++);
#if GRP_CNT >= 1
        *pDst++ = (*pSrcA++) + (*pSrcB++);
#if GRP_CNT >= 2
        *pDst++ = (*pSrcA++) + (*pSrcB++);
        *pDst++ = (*pSrcA++) + (*pSrcB++);
#if GRP_CNT >= 3
        *pDst++ = (*pSrcA++) + (*pSrcB++);
        *pDst++ = (*pSrcA++) + (*pSrcB++);
        *pDst++ = (*pSrcA++) + (*pSrcB++);
        *pDst++ = (*pSrcA++) + (*pSrcB++);
#if GRP_CNT >= 4
	#error "unsupport"
#endif
#endif
#endif
#endif

        blkCnt--;
    }

#if GRP_CNT >= 1

    blkCnt = blockSize & ((1u << GRP_CNT) - 1u);  // %

    while (blkCnt > 0)
    {
        *pDst++ = (*pSrcA++) + (*pSrcB++);
        blkCnt--;
    }

#endif
}
```

