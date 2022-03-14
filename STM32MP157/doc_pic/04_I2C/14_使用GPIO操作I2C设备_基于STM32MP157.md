## 使用GPIO操作I2C设备_STM32MP157

参考资料：

* i2c_spec.pdf
* Linux文档
  * `Linux-5.4\Documentation\devicetree\bindings\i2c\i2c-gpio.yaml`
  * `Linux-4.9.88\Documentation\devicetree\bindings\i2c\i2c-gpio.txt`
* Linux驱动源码
  * `Linux-5.4\drivers\i2c\busses\i2c-gpio.c`
  * `Linux-4.9.88\drivers\i2c\busses\i2c-gpio.c`
* 扩展板原理图：`100ASK_STM32MP157_Extend_Board.pdf`
* 本节对应的代码：
  * `doc_and_source_for_drivers\STM32MP157\source\A7\04_I2C\07_i2c_gpio_dts_stm32mp157`

### 1. 硬件连接

  * STM32MP157：把I2C模块接到GPIO
    ![image-20210312182808093](pic/04_I2C/065_use_gpio_for_i2c_module_stm32mp157.png)




### 2. 根据原理图编写设备树

#### 2.1 原理图

![image-20210312185215667](pic/04_I2C/067_gpios_for_i2c_stm32mp157.png)

#### 2.2 编写设备树

```SHELL
i2c_gpio_100ask {
	compatible = "i2c-gpio";
	gpios = <&gpioa 13 0 /* sda */
		 &gpiod 8 0 /* scl */
		>;
	i2c-gpio,delay-us = <5>;	/* ~100 kHz */
	#address-cells = <1>;
	#size-cells = <0>;
};
```

修改`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dts`，在根节点下添加上述代码。

### 3. 确认内核已经配置了I2C-GPIO

查看内核目录下的`.config`，如果未设置`CONFIG_I2C_GPIO`，上机实验时需要配置内核、编译I2C-GPIO驱动。

### 4. 上机实验

#### 4.1 设置工具链

* STM32MP157

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_stm32mp157_pro-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
  ```


#### 4.2 编译、替换设备树

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

    ![](pic/04_I2C/057_boot_mount.png)

* 更新设备树

  ```shell
  [root@100ask:~]# cp /mnt/stm32mp157c-100ask-512d-lcd-v1.dtb /boot/
  [root@100ask:~]# sync
  ```

* 重启开发板

#### 4.3 编译I2C-GPIO驱动

##### 1. 配置内核

在STM32MP157内核源码目录下执行`make menuconfig`命令，如下配置内核：

```shell
Device Drivers  --->
    I2C support  --->
        I2C Hardware Bus support  --->
            <M> GPIO-based bitbanging I2C      // 输入M，编译为模块        
```



##### 2. 编译模块

设置工具链后，在内核目录下执行：

```shell
make modules   // 得到 drivers/i2c/busses/i2c-gpio.ko
```



#### 5. 测试

在开发板上执行：

```shell
[root@100ask:~]# i2cdetect -l    // 加载i2c-gpio.ko前只看到3条I2C BUS
i2c-1   i2c             STM32F7 I2C(0x40013000)                 I2C adapter
i2c-2   i2c             STM32F7 I2C(0x5c002000)                 I2C adapter
i2c-0   i2c             STM32F7 I2C(0x40012000)                 I2C adapter
[root@100ask:~]#
[root@100ask:~]# insmod /mnt/i2c-gpio.ko
[  113.686871] i2c_gpio: module verification failed: signature and/or required key missing - tainting kernel
[  113.696497] gpio-13 (i2c_gpio_100ask): enforced open drain please flag it properly in DT/ACPI DSDT/board file
[  113.706017] gpio-56 (i2c_gpio_100ask): enforced open drain please flag it properly in DT/ACPI DSDT/board file
[  113.819836] i2c-gpio i2c_gpio_100ask: using lines 13 (SDA) and 56 (SCL)
[root@100ask:~]#
[root@100ask:~]# i2cdetect -l     // 加载i2c-gpio.ko后看到4条I2C BUS
i2c-3   i2c             i2c_gpio_100ask                         I2C adapter
i2c-1   i2c             STM32F7 I2C(0x40013000)                 I2C adapter
i2c-2   i2c             STM32F7 I2C(0x5c002000)                 I2C adapter
i2c-0   i2c             STM32F7 I2C(0x40012000)                 I2C adapter
[root@100ask:~]#
[root@100ask:~]# i2cdetect -y 3     // 检测到0x50的设备
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
[root@100ask:~]#
[root@100ask:~]# i2cset -f -y 3 0x50 0 0x55   // 往0地址写入0x55
[root@100ask:~]# i2cget -f -y 3 0x50 0        // 读0地址
0x55
```



