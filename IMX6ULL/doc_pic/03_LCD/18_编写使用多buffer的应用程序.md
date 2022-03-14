## 编写使用多buffer的应用程序

* 本节视频编写好的程序，在GIT仓库里
  * `IMX6ULL\source\03_LCD\14_use_multi_framebuffer`
  * `STM32MP157\source\A7\03_LCD\14_use_multi_framebuffer`

* 参考程序：应用基础课程里使用Framebuffer的精简程序
  * `IMX6ULL\source\03_LCD\14_use_multi_framebuffer\reference\07_framebuffer`
  * `STM32MP157\source\A7\03_LCD\14_use_multi_framebuffer\reference\07_framebuffer`
  
* 参考程序：使用多buffer的APP，在GIT仓库里
    * `IMX6ULL\source\03_LCD\13_multi_framebuffer_example\testcamera`
    * `STM32MP157\source\A7\03_LCD\13_multi_framebuffer_example\testcamera`



### 1. 编写一个支持单buffer、多buffer的APP

循环显示整屏幕的红、绿、蓝、黑、白。



### 2. 编译程序

#### 2.1 设置工具链

* 对于IMX6ULL

  ```shell
  export ARCH=arm
  export CROSS_COMPILE=arm-linux-gnueabihf-
  export PATH=$PATH:/home/book/100ask_imx6ull-sdk/ToolChain/gcc-linaro-6.2.1-2016.11-x86_64_arm-linux-gnueabihf/bin
  ```

* 对于STM32MP157

  ```shell
  source /home/book/100ask_stm32mp157_pro-sdk/ToolChain/openstlinux_eglfs-linux-gnueabi/environment-setup-cortexa7t2hf-neon-vfpv4-ostl-linux-gnueabi
  export ARCH=arm
  export CROSS_COMPILE=arm-ostl-linux-gnueabi-
  ```

  

#### 2.2 编译

设置好工具链后，把`14_use_multi_framebuffer`上传到Ubuntu，在该目录下执行`make`即可



### 3. 上机测试

#### 3.1 恢复内核使用自带的LCD驱动

* 恢复驱动程序：修改`drivers/video/fbdev/Makefile`，恢复内核自带的mxsfb.c，如下：

```shell
obj-$(CONFIG_FB_MXS)             += mxsfb.o
#obj-$(CONFIG_FB_MXS)             += lcd_drv.o
```

* 恢复设备树

  * 把GIT仓库中的文件复制到内核arch/arm/boo/dts目录
    * `doc_and_source_for_drivers\IMX6ULL\source\03_LCD\11_lcd_drv_imx6ull_ok\origin`

* 重新编译内核、设备树

  ```shell
  make zImage
  make dtbs
  ```

* 替换内核、设备树

  * 把编译出来的`arch/arm/boot/zImage`、`arch/arm/boot/dts/100ask_imx6ull-14x14.dtb`
  * 放到开发板的/boot目录

#### 3.2 禁止开发板自带的GUI程序

在开发板上执行以下命令：

```shell
[root@100ask:~]# mv /etc/init.d/S99myirhmi2 /etc/
[root@100ask:~]# reboot
```



#### 3.3 把测试程序放到板子上、执行

**以下命令在开发板中执行。**

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



* 复制、执行程序

  ```shell
  [root@100ask:~]# cp /mnt/multi_framebuffer_test   /bin
  [root@100ask:~]# multi_framebuffer_test single 或 multi_framebuffer_test double
  ```

  

### 4. LCD自动黑屏

为了省电，LCD在10分钟左右会自动黑屏。
如果你正在运行multi_framebuffer_test程序，可能会有如下提示(以IMX6ULL为例)：

```shell
[  961.147548] mxsfb 21c8000.lcdif: can't wait for VSYNC when fb is blank
```

这表示：当屏幕为blank(黑屏)时，无法等待VSYNC。

我们可以禁止LCD自动黑屏，执行以下命令即可：

```shell
#close lcd sleep
echo -e "\033[9;0]" > /dev/tty1
echo -e "\033[?25l"  > /dev/tty1
```

