qemu-system-arm -nographic -sd vexpress.img -M vexpress-a9 -m 512M -kernel zImage -append "init=/linuxrc root=/dev/mmcblk0p1 rw rootwait earlyprintk console=ttyAMA0" 2>/dev/null
