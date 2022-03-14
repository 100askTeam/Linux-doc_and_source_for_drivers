## 编程_GPIO使用Pinctrl

参考资料：

* Linux 5.x内核文档
  * Linux-5.4\Documentation\driver-api
  * Linux-5.4\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-5.4\drivers\gpio\gpio-74x164.c
  
* Linux 4.x内核文档
  * Linux-4.9.88\Documentation\gpio
  * Linux-4.9.88\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-4.9.88\drivers\gpio\gpio-74x164.c
  
* 本节视频代码在GIT仓库中

  ```shell
  doc_and_source_for_drivers\IMX6ULL\source\07_GPIO\04_gpio_use_pinctrl_ok
  doc_and_source_for_drivers\STM32MP157\source\A7\07_GPIO\04_gpio_use_pinctrl_ok
  ```

  

### 1. 我们要做什么

假设使用这个虚拟的GPIO Controller的pinA来控制LED：

![image-20210528163702792](pic/07_GPIO/11_virtual_gpio_led.png)

如果不想在使用GPIO引脚时，在设备树中设置Pinctrl信息，

如果想让GPIO和Pinctrl之间建立联系，

我们需要做这些事情：

#### 1.1 表明GPIO和Pinctrl间的联系

在GPIO设备树中使用`gpio-ranges`来描述它们之间的联系：

  * GPIO系统中有引脚号

  * Pinctrl子系统中也有自己的引脚号

  * 2个号码要建立映射关系

  * 在GPIO设备树中使用如下代码建立映射关系

    ```shell
    // 当前GPIO控制器的0号引脚, 对应pinctrlA中的128号引脚, 数量为12
    gpio-ranges = <&pinctrlA 0 128 12>; 
    ```

#### 1.2 解析这些联系

在GPIO驱动程序中，解析跟Pinctrl之间的联系：处理`gpio-ranges`:

  * 这不需要我们自己写代码

  * 注册gpio_chip时会自动调用

    ```c
    int gpiochip_add_data(struct gpio_chip *chip, void *data)
        status = of_gpiochip_add(chip);
    				status = of_gpiochip_add_pin_range(chip);
    
    of_gpiochip_add_pin_range
    	for (;; index++) {
    		ret = of_parse_phandle_with_fixed_args(np, "gpio-ranges", 3,
    				index, &pinspec);
    
        	pctldev = of_pinctrl_get(pinspec.np); // 根据gpio-ranges的第1个参数找到pctldev
    
            // 增加映射关系	
            /* npins != 0: linear range */
            ret = gpiochip_add_pin_range(chip,
                                         pinctrl_dev_get_devname(pctldev),
                                         pinspec.args[0],
                                         pinspec.args[1],
                                         pinspec.args[2]);
    ```


#### 1.3 编程

* 在GPIO驱动程序中，提供`gpio_chip->request`

* 在Pinctrl驱动程序中，提供`pmxops->gpio_request_enable`或`pmxops->request`



### 2. 编写设备树

```shell
pinctrl_virt: virtual_pincontroller {
	compatible = "100ask,virtual_pinctrl";
};

gpio_virt: virtual_gpiocontroller {
	compatible = "100ask,virtual_gpio";
    gpio-controller;
    #gpio-cells = <2>;
    ngpios = <4>;
    gpio-ranges = <&pinctrl_virt 0 0 4>; 
};

myled {
	compatible = "100ask,leddrv";
	led-gpios = <&gpio_virt 0 GPIO_ACTIVE_LOW>;
};
```



### 3. 编程

#### 3.1 GPIO控制器编程

gpio_chip中提供request函数：

```c
chip->request = gpiochip_generic_request;
```



#### 3.2 Pinctrl编程

```c
static const struct pinmux_ops virtual_pmx_ops = {
	.get_functions_count = virtual_pmx_get_funcs_count,
	.get_function_name = virtual_pmx_get_func_name,
	.get_function_groups = virtual_pmx_get_groups,
	.set_mux = virtual_pmx_set,
    .gpio_request_enable = virtual_pmx_gpio_request_enable,
};
```



### 4. 上机实验

#### 4.1 设置工具链

##### 1. STM32MP157

  **注意**：对于STM32MP157，以前说编译内核/驱动、编译APP的工具链不一样，其实编译APP用的工具链也能用来编译内核。

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_stm32mp157_pro-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
  ```

##### 2. IMX6ULL

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_imx6ull-sdk/ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
  ```



#### 4.2 编译、替换设备树

##### 1. STM32MP157

  * 修改`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dts`，添加如下代码：

    ```shell
    / {
       pinctrl_virt: virtual_pincontroller {
            compatible = "100ask,virtual_pinctrl";
            myled_pin: myled_pin {
                            functions = "gpio";
                            groups = "pin0";
                            configs = <0x11223344>;
            };
            i2cgrp: i2cgrp {
                    functions = "i2c", "i2c";
                    groups = "pin0", "pin1";
                    configs = <0x11223344  0x55667788>;
            };
        };
    
        gpio_virt: virtual_gpiocontroller {
            compatible = "100ask,virtual_gpio";
            gpio-controller;
            #gpio-cells = <2>;
            ngpios = <4>;
            gpio-ranges = <&pinctrl_virt 0 0 4>; 
        };
    
        myled {
            compatible = "100ask,leddrv";
            led-gpios = <&gpio_virt 2 GPIO_ACTIVE_LOW>;
        };
    };
    ```

    

  * 编译设备树：
    在Ubuntu的STM32MP157内核目录下执行如下命令,
    得到设备树文件：`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dtb`

    ```shell
    make dtbs
    ```

  * 复制到NFS目录：

    ```shell
    $ cp arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dtb ~/nfs_rootfs/
    ```

  * 开发板上挂载NFS文件系统

    * vmware使用NAT(假设windowsIP为192.168.1.100)

      ```shell
      [root@100ask:~]# mount -t nfs -o nolock,vers=3,port=2049,mountport=9999 
      192.168.1.100:/home/book/nfs_rootfs /mnt
      ```

    * vmware使用桥接，或者不使用vmware而是直接使用服务器：假设Ubuntu IP为192.168.1.137

      ```shell
      [root@100ask:~]#  mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
      ```

* 确定设备树分区挂载在哪里

  由于版本变化，STM32MP157单板上烧录的系统可能有细微差别。
  在开发板上执行`cat /proc/mounts`后，可以得到两种结果(见下图)：

  * mmcblk2p2分区挂载在/boot目录下(下图左边)：无需特殊操作，下面把文件复制到/boot目录即可

  * mmcblk2p2挂载在/mnt目录下(下图右边)

    * 在视频里、后面文档里，都是更新/boot目录下的文件，所以要先执行以下命令重新挂载：
      * `mount  /dev/mmcblk2p2  /boot`

    ![](../04_I2C/pic/04_I2C/057_boot_mount.png)

  * 更新设备树

    ```shell
    [root@100ask:~]# cp /mnt/stm32mp157c-100ask-512d-lcd-v1.dtb /boot
    [root@100ask:~]# sync
    ```

* 重启开发板

  



##### 2. IMX6ULL

  * 修改`arch/arm/boot/dts/100ask_imx6ull-14x14.dts`，添加如下代码：

    ```shell
    / {
       pinctrl_virt: virtual_pincontroller {
            compatible = "100ask,virtual_pinctrl";
            myled_pin: myled_pin {
                            functions = "gpio";
                            groups = "pin0";
                            configs = <0x11223344>;
            };
            i2cgrp: i2cgrp {
                    functions = "i2c", "i2c";
                    groups = "pin0", "pin1";
                    configs = <0x11223344  0x55667788>;
            };
        };
    
        gpio_virt: virtual_gpiocontroller {
            compatible = "100ask,virtual_gpio";
            gpio-controller;
            #gpio-cells = <2>;
            ngpios = <4>;
            gpio-ranges = <&pinctrl_virt 0 0 4>; 
        };
    
        myled {
            compatible = "100ask,leddrv";
            led-gpios = <&gpio_virt 2 GPIO_ACTIVE_LOW>;
        };
    };
    ```

    

  * 编译设备树：
    在Ubuntu的IMX6ULL内核目录下执行如下命令,
    得到设备树文件：`arch/arm/boot/dts/100ask_imx6ull-14x14.dtb`

    ```shell
    make dtbs
    ```

  * 复制到NFS目录：

    ```shell
    $ cp arch/arm/boot/dts/100ask_imx6ull-14x14.dtb ~/nfs_rootfs/
    ```

* 开发板上挂载NFS文件系统

  * vmware使用NAT(假设windowsIP为192.168.1.100)

    ```shell
    [root@100ask:~]# mount -t nfs -o nolock,vers=3,port=2049,mountport=9999 
    192.168.1.100:/home/book/nfs_rootfs /mnt
    ```

  * vmware使用桥接，或者不使用vmware而是直接使用服务器：假设Ubuntu IP为192.168.1.137

    ```shell
    [root@100ask:~]#  mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
    ```

  * 更新设备树

    ```shell
    [root@100ask:~]# cp /mnt/100ask_imx6ull-14x14.dtb /boot
    [root@100ask:~]# sync
    ```

* 重启开发板



#### 4.3 编译、安装驱动程序

* 编译：

  * 在Ubuntu上
  * 进入`04_gpio_use_pinctrl_ok`下的3个驱动目录，都执行make命令

* 安装：

  * 在开发板上

  * 挂载NFS，复制文件，insmod，类似如下命令：

    ```shell
    mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
    // 对于IMX6ULL，想看到驱动打印信息，需要先执行
    echo "7 4 1 7" > /proc/sys/kernel/printk
    
    insmod -f /mnt/virtual_pinctrl_driver.ko
    insmod -f /mnt/virtual_gpio_driver.ko
    insmod -f /mnt/leddrv.ko
    
    ls /dev/100ask_led0
    /mnt/ledtest /dev/100ask_led0 on
    /mnt/ledtest /dev/100ask_led0 off
    ```

* 观察内核打印的信息



### 5. 再次开后门

在STM32MP157的内核中，

Pinctrl驱动中并没有提供`pmxops->gpio_request_enable`或`pmxops->request`，

为什么也可一直接使用GPIO功能？

它的gpio_chip结构体中，有`direction_input`、`direction_output`，这2个函数的调用关系如下:

```c
direction_output/direction_input
    pinctrl_gpio_direction
    	ret = pinmux_gpio_direction(pctldev, range, pin, input);
					ret = ops->gpio_set_direction(pctldev, range, pin, input);
								stm32_pmx_gpio_set_direction
                                    stm32_pmx_set_mode  // 它会设置引脚为GPIO功能                  
```



### 6. IMX6ULL的特殊情况

IMX6ULL使用GPIO时必须设置Pinctrl，如果不设置，只有那些默认就是GPIO功能的引脚可以正常使用。

原因：

* GPIO控制器的设备树中，没有`gpio-ranges`
* Pinctrl驱动中并没有提供`pmxops->gpio_request_enable`或`pmxops->request`
* gpio_chip结构体中`direction_input`、`direction_output`，并没有配置引脚为GPIO功能