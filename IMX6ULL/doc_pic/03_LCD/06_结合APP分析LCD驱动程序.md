## 结合APP分析LCD驱动程序

本节视频对应源码在GIT仓库中，位置如下(这2个文件是完全一样的)：

```shell
doc_and_source_for_drivers\STM32MP157\source\A7\03_LCD\04_fb_test
或：
doc_and_source_for_drivers\IMX6ULL\source\03_LCD\04_fb_test
```



### 1. open

```shell
app:  open("/dev/fb0", ...)   主设备号: 29, 次设备号: 0
--------------------------------------------------------------
kernel:
         fb_open   // fbmem.c
         	struct fb_info *info;
         	info = get_fb_info(fbidx);
			
			if (info->fbops->fb_open) {
				res = info->fbops->fb_open(info,1);   // 硬件相关的驱动
				if (res)
					module_put(info->fbops->owner);
			}         	
```



### 2. 获得可变信息(含有分辨率等)

```shell
app:  	ioctl(fd, FBIOGET_VSCREENINFO, &fb_info->var);
-------------------------------------------------------------------------
kernel:
         fb_ioctl   // fbmem.c
         	struct fb_info *info = file_fb_info(file);

         	do_fb_ioctl(info, cmd, arg);
         		var = info->var;     // 硬件相关的驱动设置的
         		ret = copy_to_user(argp, &var, sizeof(var)) ? -EFAULT : 0;
```

### 3. 获得固定信息(含有显存信息)

```shell
app:  	ioctl(fd, FBIOGET_FSCREENINFO, &fb_info->fix);
-------------------------------------------------------------------------
kernel:
         fb_ioctl   // fbmem.c
         	struct fb_info *info = file_fb_info(file);

         	do_fb_ioctl(info, cmd, arg);
         		fix = info->fix;     // 硬件相关的驱动设置的
         		ret = copy_to_user(argp, &fix, sizeof(fix)) ? -EFAULT : 0;
```



### 4. mmap

```shell
app：void *ptr = mmap(0,
			fb_info->var.yres_virtual * fb_info->fix.line_length,
			PROT_WRITE | PROT_READ,
			MAP_SHARED, fd, 0);
-------------------------------------------------------------------------
kernel:
         fb_mmap   // fbmem.c
         	struct fb_info *info = file_fb_info(file);

         	start = info->fix.smem_start;
         	len = info->fix.smem_len;
         	return vm_iomap_memory(vma, start, len);
```



### 5. 绘制图片

