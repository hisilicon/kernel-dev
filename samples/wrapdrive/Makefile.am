ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign subdir-objects
AM_CFLAGS=-Wall -O0 -fno-strict-aliasing

lib_LTLIBRARIES=libwd.la
libwd_la_SOURCES=wd.c wd_util.c wd_adapter.c wd.h wd_adapter.h\
		 wd_cipher.c wd_cipher.h \
		 wd_comp.c wd_comp.h \
		 wd_dh.c wd_dh.h \
		 wd_rsa.c wd_rsa.h \
		 wd_bn.c wd_bn.h \
		 wd_dummy.c wd_dummy.h \
		 drv/hisi_sec_udrv_v1.c drv/hisi_sec_udrv_v1.h \
		 drv/dummy_drv.c drv/dummy_drv.h \
		 drv/hisi_zip_udrv.c drv/hisi_zip_udrv.h \
		 drv/hisi_hpre_udrv.c drv/hisi_hpre_udrv.h

bin_PROGRAMS=wd_clean
wd_clean_SOURCES=wd_clean.c
wd_clean_LDADD=.libs/libwd.a


SUBDIRS=. test
