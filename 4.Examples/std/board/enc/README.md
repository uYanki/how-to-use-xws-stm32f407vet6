霍尔编码器（相当于精度非常低的绝对值编码器）：

| 霍尔真值   | 001  | 010  | 011  | 100  | 101  | 110  |
| ---------- | ---- | ---- | ---- | ---- | ---- | ---- |
| **电角度** | 300  | 160  | 240  | 60   | 0    | 120  |

霍尔真值为 5 (101) 时, 表示转子 d 轴与定子 alpha 轴重合，即电角度为 0。

---

带霍尔的增量式编码器电机：第一圈使用霍尔闭环转，配到 z 轴脉冲时再切到增量。

