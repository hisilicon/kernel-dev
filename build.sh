#!/bin/sh
usage()
{
	echo "$0 board_type"
	echo "Example:	"
	echo "		$0 d01|d02|d03|d05|d06"
	exit 1
}
if [ $# != 1 ];then
	usage
fi
export board_type=$1
gen_config()
{
	case $board_type in
	"d01")
		make hisi_defconfig
	;;
	*)
		./scripts/kconfig/merge_config.sh -m arch/arm64/configs/defconfig arch/arm64/configs/plinth-config
	;;
	esac
	mv -f .config .merged.config
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- KCONFIG_ALLCONFIG=.merged.config alldefconfig
}

build_image()
{
	if [ "$board_type" = "d01" ];then
		export ARCH=arm
		export CROSS_COMPILE=arm-linux-gnueabihf-
		make -j14 zImage
		make hip04-d01.dtb
		cat arch/arm/boot/zImage arch/arm/boot/dts/hip04-d01.dtb >.kernel
	else
		export ARCH=arm64
		export CROSS_COMPILE=aarch64-linux-gnu-
		make -j16
	fi
}

build_dtb()
{
	echo "Building dtb ..."
	case $board_type in
	"d02")
	make hisilicon/hip05-d02.dtb
	;;
	"d03")
	make hisilicon/hip06-d03.dtb
	;;
	"d05")
	make hisilicon/hip07-d05.dtb
	;;
	*)
	echo "Unsupported dtb for board $board_type"
	;;
	esac
}

gen_config
build_image
build_dtb
