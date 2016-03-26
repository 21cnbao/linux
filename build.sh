export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
make zImage uImage dtbs modules LOADADDR=0x40008000 LOCALVERSION= -j4

cp arch/arm/boot/zImage ../images/
cp arch/arm/boot/uImage ../images/
cp arch/arm/boot/dts/sun7i-a20-pcduino3-nano.dtb ../images
cat ../images/zImage ../images/sun7i-a20-pcduino3-nano.dtb > ../images/zImage.dtb
mkimage -A arm -O linux -T kernel -C none -a 40008000 -e 40008000 -n linux -d ../images/zImage.dtb ../images/uImage.dtb

sudo make ARCH=arm modules_install INSTALL_MOD_PATH=../images/rootfs LOCALVERSION= -j4
