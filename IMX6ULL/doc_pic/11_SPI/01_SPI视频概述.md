# SPI视频概述 #



# 1. SPI硬件框架

![image-20220216103153006](pic/01_hardware_block.jpg)



## 2. SPI视频涉及的内容

* SPI协议
* SPI驱动程序框架
  * SPI总线设备驱动模型
  * SPI设备树处理过程
* 简单的SPI设备驱动
  * SPI ADC
  * SPI OLED
  * 内核自带的spi dev驱动

* 复杂的SPI设备驱动
  * SPI+FrameBuffer
* SPI控制器驱动程序
  * 使用GPIO实现
  * 具体芯片的SPI控制器驱动程序分析

* SPI调试工具
  * spi-tools
* 高性能：QSPI
* 主控芯片用作SPI从设备



录制视频时留意这些知识点：

* SPI3线和SPI4线
* bits_per_word设置8和16会影响到啥
* 硬件片选和软件片选在驱动咋用

