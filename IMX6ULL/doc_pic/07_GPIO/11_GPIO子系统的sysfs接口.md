## GPIO子系统的sysfs接口

参考资料：

* Linux 5.x内核文档
  * Linux-5.4\Documentation\driver-api
  * Linux-5.4\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-5.4\drivers\gpio\gpiolib-sysfs.c
  
* Linux 4.x内核文档
  * Linux-4.9.88\Documentation\gpio
  * Linux-4.9.88\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-4.9.88\drivers\gpio\gpiolib-sysfs.c
  
* 本节视频代码在GIT仓库中

  ```shell
  doc_and_source_for_drivers\IMX6ULL\source\07_GPIO\04_gpio_use_pinctrl_ok
  doc_and_source_for_drivers\STM32MP157\source\A7\07_GPIO\04_gpio_use_pinctrl_ok
  ```

  

### 1. 驱动程序

驱动程序为`drivers\gpio\gpiolib-sysfs.c`，这里不打算分析它。



### 2. 常用的SYSFS文件

#### 2.1 有哪些GPIO控制器

`/sys/bus/gpio/devices`目录下，列出了所有的GPIO控制器，如下表示有11个GPIO控制器：

```c
/sys/bus/gpio/devices/gpiochip0
/sys/bus/gpio/devices/gpiochip1
/sys/bus/gpio/devices/gpiochip2
/sys/bus/gpio/devices/gpiochip3
/sys/bus/gpio/devices/gpiochip4
/sys/bus/gpio/devices/gpiochip5
/sys/bus/gpio/devices/gpiochip6
/sys/bus/gpio/devices/gpiochip7
/sys/bus/gpio/devices/gpiochip8
/sys/bus/gpio/devices/gpiochip9
/sys/bus/gpio/devices/gpiochip10
```



#### 2.2 每个GPIO控制器的详细信息

`/sys/class/gpio/gpiochipXXX`下，有这些信息：

```shell
/sys/class/gpio/gpiochip508]# ls -1
base     // 这个GPIO控制器的GPIO编号
device
label    // 名字
ngpio    // 引脚个数
power
subsystem
uevent
```



#### 2.3 查看GPIO使用情况

```shell
cat /sys/kernel/debug/gpio
```



#### 2.4 通过SYSFS使用GPIO

如果只是简单的引脚控制(比如输出、查询输入值)，可以不编写驱动程序。

但是涉及中断的话，就需要编写驱动程序了。

##### 1. 确定GPIO编号

查看每个`/sys/bus/gpio/devices/gpiochipXXX`目录下的label，确定是你要用的GPIO控制器，也称为GPIO Bank。

根据它名字gpiochipXXX，就可以知道基值是XXX。

基值加上引脚offset，就是这个引脚的编号。



##### 2. 导出/设置方向/读写值

举例：

```shell
echo 509 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio509/direction
echo 1 > /sys/class/gpio/gpio509/value
echo 509 > /sys/class/gpio/unexport

echo 509 > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio509/direction
cat /sys/class/gpio/gpio509/value
echo 509 > /sys/class/gpio/unexport
```



