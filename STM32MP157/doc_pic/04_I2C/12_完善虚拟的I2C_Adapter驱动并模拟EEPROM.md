## 完善虚拟的I2C\_Adapter驱动并模拟EEPROM

参考资料：

* Linux内核文档:
  * `Linux-4.9.88\Documentation\devicetree\bindings\i2c\i2c-gpio.txt`
  * `Linux-5.4\Documentation\devicetree\bindings\i2c\i2c-gpio.yaml`
  
* Linux内核驱动程序：使用GPIO模拟I2C
  
  * `Linux-4.9.88\drivers\i2c\busses\i2c-gpio.c`
  * `Linux-5.4\drivers\i2c\busses\i2c-gpio.c`
  
* Linux内核真正的I2C控制器驱动程序

  * IMX6ULL: `Linux-4.9.88\drivers\i2c\busses\i2c-imx.c`
* STM32MP157: `Linux-5.4\drivers\i2c\busses\i2c-stm32f7.c`
* 本节视频的代码在GIT仓库里
  * IMX6ULL：`doc_and_source_for_drivers\IMX6ULL\source\04_I2C\06_i2c_adapter_virtual_ok`
  * STM32MP157：`doc_and_source_for_drivers\STM32MP157\source\A7\04_I2C\06_i2c_adapter_virtual_ok`

### 1. 实现master_xfer函数

在虚拟的I2C\_Adapter驱动程序里，只要实现了其中的master_xfer函数，这个I2C Adapter就可以使用了。
在master_xfer函数里，我们模拟一个EEPROM，思路如下：

* 分配一个512自己的buffer，表示EEPROM
* 对于slave address为0x50的i2c_msg，解析并处理
  * 对于写：把i2c_msg的数据写入buffer
  * 对于读：从buffer中把数据写入i2c_msg
* 对于slave address为其他值的i2c_msg，返回错误



### 2. 编程

看视频



### 3. 上机实验

#### 3.1 设置工具链

* IMX6ULL

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_imx6ull-sdk/ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
  ```

* STM32MP157
  **注意**：对于STM32MP157，以前说编译内核/驱动、编译APP的工具链不一样，其实编译APP用的工具链也能用来编译内核。

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_stm32mp157_pro-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
  ```


#### 3.2 编译、替换设备树

在设备树根节点下，添加如下代码：

```shell
	i2c-bus-virtual {
		 compatible = "100ask,i2c-bus-virtual";
	};
```

##### 1. STM32MP157

  * 修改`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dts`，添加如下代码：

    ```shell
    / {
    	i2c-bus-virtual {
    		 compatible = "100ask,i2c-bus-virtual";
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

      ![](pic/04_I2C/057_boot_mount.png)

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
    	i2c-bus-virtual {
    		 compatible = "100ask,i2c-bus-virtual";
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



#### 3.4 编译、安装驱动程序

* 编译：

  * 在Ubuntu上
  * 修改`06_i2c_adapter_virtual_ok`中的Makefile，指定内核路径`KERN_DIR`，在执行`make`命令即可。

* 安装：

  * 在开发板上

  * 挂载NFS，复制文件，insmod，类似如下命令：

    ```shell
    mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
    // 对于IMX6ULL，想看到驱动打印信息，需要先执行
    echo "7 4 1 7" > /proc/sys/kernel/printk
    
    insmod /mnt/i2c_adapter_drv.ko
    ```

    

  

#### 3.5 使用i2c-tools测试

在开发板上执行，命令如下：

* 列出I2C总线

  ```shell
  i2cdetect -l
  ```

  结果类似下列的信息：

  ```shell
  i2c-1   i2c             21a4000.i2c                             I2C adapter
  i2c-4   i2c             i2c-bus-virtual                         I2C adapter
  i2c-0   i2c             21a0000.i2c                             I2C adapter
  ```

  **注意**：不同的板子上，i2c-bus-virtual的总线号可能不一样，上问中总线号是4。

  

* 检查虚拟总线下的I2C设备

  ```shell
  // 假设虚拟I2C BUS号为4
  [root@100ask:~]# i2cdetect -y -a 4
       0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
  00: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  50: 50 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  70: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
  ```

  

* 读写模拟的EEPROM

  ```shell
  // 假设虚拟I2C BUS号为4
  [root@100ask:~]# i2cset -f -y 4 0x50 0 0x55   // 往0地址写入0x55
  [root@100ask:~]# i2cget -f -y 4 0x50 0        // 读0地址
  0x55
  ```

  


