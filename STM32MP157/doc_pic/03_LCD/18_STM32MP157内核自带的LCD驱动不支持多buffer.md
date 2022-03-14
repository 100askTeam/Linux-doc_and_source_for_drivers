## STM32MP157内核自带的LCD驱动不支持多buffer

* 本节视频编写好的程序，在GIT仓库里
  * `IMX6ULL\source\03_LCD\14_use_multi_framebuffer`
  * `STM32MP157\source\A7\03_LCD\14_use_multi_framebuffer`

* 参考程序：应用基础课程里使用Framebuffer的精简程序
  * `IMX6ULL\source\03_LCD\14_use_multi_framebuffer\reference\07_framebuffer`
  * `STM32MP157\source\A7\03_LCD\14_use_multi_framebuffer\reference\07_framebuffer`
  
* 参考程序：使用多buffer的APP，在GIT仓库里
    * `IMX6ULL\source\03_LCD\13_multi_framebuffer_example\testcamera`
    * `STM32MP157\source\A7\03_LCD\13_multi_framebuffer_example\testcamera`



### 1. STM32MP157内核自带的LCD驱动不支持都buffer

所以无法在STM32MP157观察到多buffer的效果。
如果想学习多buffer的APP如何编写，请学习IMX6ULL的视频：`18_编写使用多buffer的应用程序`



### 2. 恢复自带的驱动(不想那么复杂的话，直接重烧系统)

**注意**：

* 一旦使用重新编译的内核，板子自带的GUI无法运行

* 原因在于内核重新编译后，也需要重新编译、安装各类模块，还有第3方模块。

* 编译第3方模块涉及buildroot的使用

* 这些不是LCD的内容，所以建议还是直接通过USB恢复系统

  

#### 2.1 在Ubuntu上编译内核、设备树

* 设置工具链，执行如下命令：
  
```shell
source /home/book/100ask_stm32mp157_pro-sdk/ToolChain/openstlinux_eglfs-linux-gnueabi/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
export ARCH=arm
export CROSS_COMPILE=arm-ostl-linux-gnueabi-
```

  

* 去掉自己编写的驱动程序
  * 修改内核文件：`drivers/video/fbdev/Makefile`，如下注释掉lcd_drv.o那行：
  
```shell
  #obj-y             += lcd_drv.o
```

* 重新配置内核选择自带驱动程序，执行`make menuconfig`，如下配置2项：

  ```shell
  Device Drivers  --->
      Graphics support  ---> 
          <*> DRM Support for STMicroelectronics SoC Series                  // 输入Y选择
          <*>   STMicroelectronics specific extensions for Synopsys MIPI DSI // 输入Y选择
  ```

* 恢复设备树

  * 把GIT仓库中`STM32MP157\source\A7\03_LCD\11_lcd_drv_stm32mp157_ok\origin\stm32mp157c-100ask-512d-lcd-v1.dts`
  * 复制到内核目录：`arch/arm/boot/dts`目录下

* 编译内核、设备树

  ```shell
  ~/100ask_stm32mp157_pro-sdk/$ cd Linux-5.4
  ~/100ask_stm32mp157_pro-sdk/Linux-5.4$ make uImage LOADADDR=0xC2000040 
  ~/100ask_stm32mp157_pro-sdk/Linux-5.4$ make dtbs
  ```
  
* 得到
  * 内核：`arch/arm/boot/uImage`
  * 设备树文件：`arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dtb`
  
* 复制到NFS目录：

  ```shell
  $ cp arch/arm/boot/uImage ~/nfs_rootfs/
  $ cp arch/arm/boot/dts/stm32mp157c-100ask-512d-lcd-v1.dtb ~/nfs_rootfs/
  ```



#### 2.2 在开发板上通过NFS更新内核、设备树

* 挂载NFS

  * vmware使用NAT(假设windowsIP为192.168.1.100)

    ```shell
    [root@100ask:~]# mount -t nfs -o nolock,vers=3,port=2049,mountport=9999 
    192.168.1.100:/home/book/nfs_rootfs /mnt
    ```

  * vmware使用桥接，或者不使用vmware而是直接使用服务器：假设Ubuntu IP为192.168.1.137

    ```shell
    [root@100ask:~]#  mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
    ```

* 确定单板上内核、设备树保存在哪里

  由于版本变化，STM32MP157单板上烧录的系统可能有细微差别，在开发板上执行`cat /proc/mounts`后，
  可以得到两种结果(见下图)：

  * 保存内核、设备树的分区，挂载在/boot目录下：无需特殊操作
  * 保存内核、设备树的分区，挂载在/mnt目录下
    * 在视频里、后面文档里，都是更新/boot目录下的文件，所以要先执行以下命令重新挂载：
      * `mount  /dev/mmcblk2p2  /boot`


  ![image-20210203132435507](pic/02_LCD驱动/047_boot_mount.png)

* 更新单板文件

  ```shell
  [root@100ask:~]# cp /mnt/uImage /boot
  [root@100ask:~]# cp /mnt/stm32mp157c-100ask-512d-lcd-v1.dtb /boot
  ```





### 3. 编译、运行APP

* 设置工具链，执行如下命令：
  
```shell
export ARCH=arm
export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
export PATH=$PATH:/home/book/100ask_stm32mp157_pro-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
```

* 上传代码、编译
  * 代码位置：GIT仓库`STM32MP157\source\A7\03_LCD\14_use_multi_framebuffer`
  * 上传到Ubuntu
  * 在`14_use_multi_framebuffer`目录下执行make，可以得到`multi_framebuffer_test`

* 通过NFS放到开发板/bin目录

  ```shell
  mount -t nfs -o nolock,vers=3 192.168.1.137:/home/book/nfs_rootfs /mnt
  cp /mnt/multi_framebuffer_test  /bin
  ```

  

* 在开发板执行

  ```shell
  systemctl stop myir  //关闭自带的GUI程序
  multi_framebuffer_test  double 或 multi_framebuffer_test single
  ```

* 由于STM32MP157自带的LCD驱动不支持多buffer，上述命令效果一样



### 4. LCD自动黑屏

为了省电，LCD在10分钟左右会自动黑屏。

我们可以禁止LCD自动黑屏，执行以下命令即可：

```shell
#close lcd sleep
echo -e "\033[9;0]" > /dev/tty1
echo -e "\033[?25l"  > /dev/tty1
```

