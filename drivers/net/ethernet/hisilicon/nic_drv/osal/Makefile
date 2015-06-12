EXTRA_CFLAGS += -DHRD_OS_LINUX -DHRD_ENDNESS_LITTLE  -DBOARD_TYPE_CHIP -I$(src)/../include -I./crg -Wno-error=date-time
ALL_DRV_OBJS = hrdCommon.o hrdOs.o crg/sre_sub_dsaf.o crg/sre_sub_pcie.o crg/sre_sub_crg.o


obj-$(CONFIG_HI_NET_OSAL) += pv660_hal.o
pv660_hal-objs := $(ALL_DRV_OBJS)

