EXTRA_CFLAGS += -DHRD_OS_LINUX -DHRD_ENDNESS_LITTLE  -DBOARD_TYPE_CHIP -I$(src)/../include -Wno-error=date-time

MDIO_OBJS := iware_mdio_main.o  iware_mdio_hal.o
obj-$(CONFIG_HI_NET_MDIO) += mdio.o
mdio-objs := $(MDIO_OBJS)

