## 链式中断控制器驱动程序编写_linear方式

参考资料：

* [linux kernel的中断子系统之（七）：GIC代码分析](http://www.wowotech.net/irq_subsystem/gic_driver.html)

* Linux 4.9.88内核源码

  * `Linux-4.9.88\drivers\gpio\gpio-mxc.c`
  * `Linux-4.9.88\arch\arm\boot\dts\imx6ull.dtsi`

* Linux 5.4内核源码
  
  * `Linux-5.4\drivers\pinctrl\stm32\pinctrl-stm32mp157.c`
  * `Linux-5.4\drivers\irqchip\irq-stm32-exti.c`
  * `Linux-5.4\arch\arm\boot\dts\stm32mp151.dtsi`
  
* 本节视频源码在GIT仓库里

  ```shell
  doc_and_source_for_drivers\
  	IMX6ULL\source\08_Interrupt\
  		05_virtual_int_controller_linear_ok
		
  doc_and_source_for_drivers\
  	STM32MP157\source\A7\08_Interrupt\
  		05_virtual_int_controller_linear_ok
  ```
  
  

### 1. 两种方式的对比

linear、legacy方式，都是用来编写链式中断控制器驱动程序，它们的关系如下表所示。

|              | legacy                                        | linear                |
| ------------ | --------------------------------------------- | --------------------- |
| 函数         | irq_domain_add_legacy                         | irq_domain_add_linear |
| irq_desc     | 一次性分配完                                  | 用到再分配            |
| (hwirq,virq) | domain->linear_revmap[hwirq] = irq_data->irq; | 同左边                |




### 2. 编程



### 3. 上机实验

跟上节视频操作完全一样，参考《16_legacy方式代码的上机实验.md》

