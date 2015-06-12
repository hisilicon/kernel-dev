EXTRA_CFLAGS += -DHRD_OS_LINUX -DHRD_ENDNESS_LITTLE  -DBOARD_TYPE_CHIP -I$(src)/../include -Wno-error=date-time

NIC_DRV_OBJS := iware_nic_main.o iware_ppe_main.o iware_rcb_main.o iware_nic_ethtool.o iware_nic_sysfs.o
NIC_HAL_OBJS  := iware_nic_hal.o iware_ppe_hal.o iware_rcb_hal.o

obj-$(CONFIG_HI_NET_NIC) += nic.o
nic-objs := $(NIC_DRV_OBJS) $(NIC_HAL_OBJS)

