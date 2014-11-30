export ARCH=arm
export EXTRADIR=${PWD}/extra
export CROSS_COMPILE=arm-linux-gnueabi-
make LDDD3_vexpress_defconfig
make zImage -j8
make modules -j8
make dtbs
cp arch/arm/boot/zImage extra/
cp arch/arm/boot/dts/*ca9.dtb	extra/
cp .config extra/
