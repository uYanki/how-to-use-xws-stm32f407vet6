![image-20231107013728924](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107013728924.png)

![image-20231107013758793](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107013758793.png)

### 修改报告描述表

![image-20231107013839259](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107013839259.png)

![image-20231107013900281](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107013900281.png)

* Input：上传数据

* Output：下传数据

* Feature：上下传数据

### 修改端点大小

![image-20231107014056703](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107014056703.png)

![image-20231107014147083](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107014147083.png)

要能最大上下传**64byte**数据，还需修改对应上下传端点大小为64。

ADDR中最高位代表端点的方向，1对应IN，0对应OUT，剩下7位为端点号，0x81代表端点1为IN Endpoint (EP1)，这里修改为OUT的端点为Endpoint 2 (EP2)，两个端点的SIZE也由2改为64。



配置USB Device的VID、PID、Product、Manufacturer等信息

![image-20231107014301508](C:\Users\uYanki\AppData\Roaming\Typora\typora-user-images\image-20231107014301508.png)