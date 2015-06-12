EXTRA_CFLAGS += -DHRD_OS_LINUX -DHRD_ENDNESS_LITTLE  -DBOARD_TYPE_CHIP -I$(src)/../include -Wno-error=date-time
MAC_OBJS = iware_led_hal.o iware_gmac_hal.o iware_xgmac_hal.o iware_mac_hal.o iware_mac_main.o

obj-$(CONFIG_HI_NET_MAC) += mac.o
mac-objs := $(MAC_OBJS)

