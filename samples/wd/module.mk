LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := warpdrive

warpdrive_get_install_lib_dir = $(call get-installed-usr-path,$(PRIVATE_INSTALL_DIR),lib)
warpdrive_get_install_bin_dir = $(call get-installed-usr-path,$(PRIVATE_INSTALL_DIR),sbin)
warpdrive_get_src_dir = $(abspath $(PRIVATE_PATH))

define warpdrive_cmd_configure
	@mkdir -p $(PRIVATE_BUILD_DIR)
	$(call update_source_map_if_needed,$(call warpdrive_get_src_dir),$(call warpdrive_get_src_dir))
endef

define warpdrive_cmd_build
	cd $(PRIVATE_PATH);./cleanup.sh;\
	mkdir m4;\
	./autogen.sh;\
	CC="$(TARGET_COMPILER_PREFIX)gcc $(call get-sc-ext-cflags, $(call warpdrive_get_src_dir))"\
	LD=$(TARGET_COMPILER_PREFIX)ld \
	./configure --host aarch64-linux-gnu --target aarch64-linux-gnu \
	 --with-openssl_dir=$(abspath $(PRIVATE_PATH))/../openssl;\
	make
endef

define warpdrive_cmd_install
	@mkdir -p $(call warpdrive_get_install_lib_dir)
	@mkdir -p $(call warpdrive_get_install_bin_dir)
	cp -rf -P $(PRIVATE_PATH)/.libs/libwd* $(call warpdrive_get_install_lib_dir)
	cp -rf $(PRIVATE_PATH)/test/test_hisi_zip $(call warpdrive_get_install_bin_dir)
	cp -rf $(PRIVATE_PATH)/test/test_hisi_zlib $(call warpdrive_get_install_bin_dir)
	cp -rf $(PRIVATE_PATH)/test/wd_zip_test $(call warpdrive_get_install_bin_dir)
	cp -rf $(PRIVATE_PATH)/test/hisi_hpre_test/test_hisi_hpre $(call warpdrive_get_install_bin_dir)
endef

define warpdrive_cmd_clean
	$(Q) rm -f $(PRIVATE_BUILD_DIR)
	$(Q) rm -f $(PRIVATE_INSTALL_DIR)
endef

LOCAL_DEF_CMD_CONFIGURE := warpdrive_cmd_configure
LOCAL_DEF_CMD_BUILD := warpdrive_cmd_build
LOCAL_DEF_CMD_INSTALL := warpdrive_cmd_install
LOCAL_DEF_CMD_CLEAN := warpdrive_cmd_clean

include $(BUILD_SYSTEM)/opensource_commom.mk
