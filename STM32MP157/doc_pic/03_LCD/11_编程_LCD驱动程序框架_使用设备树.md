## 编程\_LCD驱动程序框架\_使用设备树

参考资料，GIT仓库里：

* 基于这个程序修改：
  * `IMX6ULL\source\03_LCD\03_lcd_drv_qemu_ok`
  * `STM32MP157\source\A7\03_LCD\03_lcd_drv_qemu_ok`
  
* 参考：内核自带的示例驱动程序
  
  * Linux驱动源码：`drivers/video/fbdev/simplefb.c`
  * 设备树：
    * `arch/arm/boot/dts/sun4i-a10.dtsi`
  

* 本节视频编写好的代码
  * `IMX6ULL\source\03_LCD\06_lcd_drv_framework_use_devicetree`
  * `STM32MP157\source\A7\03_LCD\06_lcd_drv_framework_use_devicetree`

### 1. 说明

Linux驱动程序 = 驱动程序框架 + 硬件编程。
在前面已经基于QEMU编写了LCD驱动程序，对LCD驱动程序的框架已经分析清楚。
核心就是：

* 分配fb_info
* 设置fb_info
* 注册fb_info
* 硬件相关的设置

本节课程我们基于设备树来编写驱动程序。



### 2. 入口函数注册platform_driver

### 3. 设备树有对应节点

```shell

                framebuffer-mylcd {
                        compatible = "100ask,lcd_drv";
                };

```



### 4. 编写probe函数

* 分配fb_info
* 设置fb_info
* 注册fb_info
* 硬件相关的设置
  * 引脚设置
  * 时钟设置
  * LCD控制器设置




