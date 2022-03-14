## Pinctrl子系统视频介绍

参考资料：

* Linux 5.x内核文档
  * Documentation\devicetree\bindings\pinctrl\pinctrl-bindings.txt
* Linux 4.x内核文档
  * Documentation\pinctrl.txt
  * Documentation\devicetree\bindings\pinctrl\pinctrl-bindings.txt

### 1. Pinctrl作用

![image-20210430121123225](pic/06_Pinctrl/06_pinctrl_hardware_block.png)

Pinctrl：Pin Controller，顾名思义，就是用来控制引脚的：

* 引脚枚举与命名(Enumerating and naming)
* 引脚复用(Multiplexing)：比如用作GPIO、I2C或其他功能
* 引脚配置(Configuration)：比如上拉、下来、open drain、驱动强度等



Pinctrl驱动由芯片厂家的BSP工程师提供，一般的驱动工程师只需要在设备树里：

* 指明使用那些引脚
* 复用为哪些功能
* 配置为哪些状态

在一般的设备驱动程序里，甚至可以没有pinctrl的代码。

对于一般的驱动工程师，只需要知道“怎么使用pinctrl”即可。



### 2. 预计录制的内容

* 基于使用的角度驱动工程师要掌握的Pinctrl重要概念

* Pinctrl子系统使用示例

* 主要数据结构与调试方法

* pincontroller的数据结构构造过程情景分析(会拆分为很多节)

  * 根据开发板的设备树信息动态地演示

* client端的数据结构构造过程情景分析(会拆分为很多节)

  * 根据开发板的设备树信息动态地演示

* 编写一个虚拟的pincontroller驱动程序

  