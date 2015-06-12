EXTRA_CFLAGS += -DHRD_OS_LINUX -DHRD_ENDNESS_LITTLE  -DBOARD_TYPE_CHIP -I$(src)/../include -Wno-error=date-time

DSAF_OBJS = iware_dsaf_drv_hw.o iware_dasf_drv_hal.o iware_dsaf_sysfs.o iware_dsaf_main.o

obj-$(CONFIG_HI_NET_DSAF) += dsaf.o
dsaf-objs := $(DSAF_OBJS)
