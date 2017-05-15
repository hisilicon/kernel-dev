#!/bin/bash

./scripts/kconfig/merge_config.sh -m arch/arm64/configs/defconfig arch/arm64/configs/plinth-config 
mv -f .config .merged.config

make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- KCONFIG_ALLCONFIG=.merged.config alldefconfig
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu-   Image -j40

#scp -r arch/arm64/boot/Image kongxinwei@192.168.1.107:/home/hisilicon/ftp/kongxinwei/Image_d05
