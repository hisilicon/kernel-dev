#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- distro.config
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- defconfig
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- Image -j32
#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- modules -j32
#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- -j32
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hip05-d02.dtb -j32
#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hisi_p660_evb.dtb -j32
#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hisi_hi1610_chip_evb.dtb -j32
#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hi1612-evb.dtb -j32
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hip06-d03.dtb -j32
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hip06_32c-evb.dtb -j32
make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hip07-d05.dtb -j32


make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- hisilicon/hip07-d05.dtb -j32

#make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- -C ~/kernel-dev/


make ARCH=arm64 CROSS_COMPILE=~/Downloads/gcc_arm_8.2_be/gcc-arm-8.2-2018.08-x86_64-aarch64_be-linux-gnu/bin/aarch64_be-linux-gnu- -C tools/perf

