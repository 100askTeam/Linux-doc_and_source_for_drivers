## 调试与使用虚拟的GPIO控制器

参考资料：

* Linux 5.x内核文档
  * Linux-5.4\Documentation\driver-api
  * Linux-5.4\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-5.4\drivers\gpio\gpio-74x164.c
  
* Linux 4.x内核文档
  * Linux-4.9.88\Documentation\gpio
  * Linux-4.9.88\Documentation\devicetree\bindings\gpio\gpio.txt
  * Linux-4.9.88\drivers\gpio\gpio-74x164.c
  
* 本章课程源码位于GIT仓库里

  ```shell
  doc_and_source_for_drivers\IMX6ULL\source\07_GPIO\03_virtual_gpio_ok
  doc_and_source_for_drivers\STM32MP157\source\A7\07_GPIO\03_virtual_gpio_ok
  ```

  

### 1. 硬件功能

假设使用这个虚拟的GPIO Controller的pinA来控制LED：

![image-20210528163702792](pic/07_GPIO/11_virtual_gpio_led.png)



### 2. 编写设备树文件

```shell
gpio_virt: virtual_gpiocontroller {
	compatible = "100ask,virtual_gpio";
    gpio-controller;
    #gpio-cells = <2>;
    ngpios = <4>;
};

myled {
	compatible = "100ask,leddrv";
	led-gpios = <&gpio_virt 0 GPIO_ACTIVE_LOW>;
};
```



### 3. 上机实验

#### 3.1 设置工具链

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



#### 3.2 编译、替换设备树

##### 1. STM32MP157

  * 修改`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dts`，添加如下代码：

    ```shell
    / {
        gpio_virt: virtual_gpiocontroller {
            compatible = "100ask,virtual_gpio";
            gpio-controller;
            #gpio-cells = <2>;
            ngpios = <4>;
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
        gpio_virt: virtual_gpiocontroller {
            compatible = "100ask,virtual_gpio";
            gpio-controller;
            #gpio-cells = <2>;
            ngpios = <4>;
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



#### 3.3 编译、安装驱动程序

* 编译：

  * 在Ubuntu上
  * 修改`01_led`、`02_virtual_gpio`中的Makefile，指定内核路径`KERN_DIR`，在执行`make`命令即可。

* 安装：

  * 在开发板上

  * 挂载NFS，复制文件，insmod，类似如下命令：

    ```shell
    mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
    // 对于IMX6ULL，想看到驱动打印信息，需要先执行
    echo "7 4 1 7" > /proc/sys/kernel/printk
    
    insmod -f /mnt/virtual_gpio_driver.ko
    insmod -f /mnt/leddrv.ko
    
    ls /dev/100ask_led0
    /mnt/ledtest /dev/100ask_led0 on
    /mnt/ledtest /dev/100ask_led0 off
    ```
* 观察内核打印的信息
  



### 4. STM32MP157上的bug

在STM32MP157上做如下实验时：

```shell
echo 509 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio509/direction
cat /sys/class/gpio/gpio509/value
```

发现对于`value`执行一次cat操作，导致`virt_gpio_get_value`函数被调用3次，如下：

```shell
cat /sys/class/gpio/gpio509/value
[   96.283263] get pin 1, it's val = 0
[   96.297803] get pin 1, it's val = 0
[   96.312604] get pin 1, it's val = 0
```

`cat value`这个操作，会导致驱动`drivers/gpio/gpiolib-sysfs.c`中的value_show函数被调用。

value_show只会调用一次GPIO Controller中的get函数。

所以，我们编写了一个read.c程序，源码如下：

```c
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
        int fd = open(argv[1], O_RDONLY);
    
        char buf[10];
    
        read(fd, buf, 10);
        printf("%s\n", buf);
        return 0;
}
```

编译read.c:

```shell
arm-buildroot-linux-gnueabihf-gcc -o read read.c
```

放到板子上执行，发现读取value文件一次，只会导致get函数被调用一次，如下：

```shell
# ./read /sys/class/gpio/gpio509/value
[  298.663613] get pin 1, it's val = 0
1
```



所以：问题在于cat命令，虽然我们执行了一次cat操作，但是它发起了3次读value文件的操作。

至于cat的bug在哪，无关紧要，先不花时间去查。