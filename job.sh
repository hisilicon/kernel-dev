#!/bin/sh

export_top_env()
{
	export suite='stress-ng'
	export testcase='stress-ng'
	export category='benchmark'
	export nr_threads=9
	export testtime=60
	export job_origin='stress-ng-class-filesystem.yaml'
	export queue_cmdline_keys=
	export queue='vip'
	export testbox='lkp-icl-2sp1'
	export tbox_group='lkp-icl-2sp1'
	export submit_id='62e7dc320b9a9362f3b4f2e8'
	export job_file='/lkp/jobs/scheduled/lkp-icl-2sp1/stress-ng-filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280-debian-11.1-x86_64-20220510.cgz-0568e6122574dcc1ade-20220801-25331-42q02g-3.yaml'
	export id='3fbc572da5a605559d4aefc7447df50796d1604b'
	export queuer_version='/lkp/xsang/.src-20220801-163609'
	export model='Ice Lake'
	export nr_node=2
	export nr_cpu=96
	export memory='256G'
	export nr_hdd_partitions=1
	export hdd_partitions='/dev/disk/by-id/ata-ST9500530NS_9SP1KLAR-part1'
	export ssd_partitions='/dev/nvme0n1p1'
	export swap_partitions=
	export kernel_cmdline_hw='acpi_rsdp=0x667fd014'
	export rootfs_partition='/dev/disk/by-id/ata-INTEL_SSDSC2BB800G4_PHWL4204005K800RGN-part3'
	export need_kconfig='BLK_DEV_SD
SCSI
{"BLOCK"=>"y"}
SATA_AHCI
SATA_AHCI_PLATFORM
ATA
{"PCI"=>"y"}
F2FS_FS'
	export commit='0568e6122574dcc1aded2979cd0245038efe22b6'
	export ucode='0xb000280'
	export need_kconfig_hw='{"PTP_1588_CLOCK"=>"y"}
{"IGB"=>"y"}
{"IXGBE"=>"y"}
SATA_AHCI'
	export bisect_dmesg=true
	export kconfig='x86_64-rhel-8.3'
	export enqueue_time='2022-08-01 21:59:14 +0800'
	export _id='62e7dc350b9a9362f3b4f2eb'
	export _rt='/result/stress-ng/filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280/lkp-icl-2sp1/debian-11.1-x86_64-20220510.cgz/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6'
	export user='lkp'
	export compiler='gcc-11'
	export LKP_SERVER='internal-lkp-server'
	export head_commit='75648c1f8e7a2175b491072c438b0edd18c5f7cc'
	export base_commit='e0dccc3b76fb35bb257b4118367a883073d7390e'
	export branch='hch-dma-mapping/for-next'
	export rootfs='debian-11.1-x86_64-20220510.cgz'
	export result_root='/result/stress-ng/filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280/lkp-icl-2sp1/debian-11.1-x86_64-20220510.cgz/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6/8'
	export scheduler_version='/lkp/lkp/.src-20220728-201011'
	export arch='x86_64'
	export max_uptime=2100
	export initrd='/osimage/debian/debian-11.1-x86_64-20220510.cgz'
	export bootloader_append='root=/dev/ram0
RESULT_ROOT=/result/stress-ng/filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280/lkp-icl-2sp1/debian-11.1-x86_64-20220510.cgz/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6/8
BOOT_IMAGE=/pkg/linux/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6/vmlinuz-5.19.0-rc3-00027-g0568e6122574
branch=hch-dma-mapping/for-next
job=/lkp/jobs/scheduled/lkp-icl-2sp1/stress-ng-filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280-debian-11.1-x86_64-20220510.cgz-0568e6122574dcc1ade-20220801-25331-42q02g-3.yaml
user=lkp
ARCH=x86_64
kconfig=x86_64-rhel-8.3
commit=0568e6122574dcc1aded2979cd0245038efe22b6
acpi_rsdp=0x667fd014
max_uptime=2100
LKP_SERVER=internal-lkp-server
nokaslr
selinux=0
debug
apic=debug
sysrq_always_enabled
rcupdate.rcu_cpu_stall_timeout=100
net.ifnames=0
printk.devkmsg=on
panic=-1
softlockup_panic=1
nmi_watchdog=panic
oops=panic
load_ramdisk=2
prompt_ramdisk=0
drbd.minor_count=8
systemd.log_level=err
ignore_loglevel
console=tty0
earlyprintk=ttyS0,115200
console=ttyS0,115200
vga=normal
rw'
	export modules_initrd='/pkg/linux/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6/modules.cgz'
	export bm_initrd='/osimage/deps/debian-11.1-x86_64-20220510.cgz/run-ipconfig_20220515.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/lkp_20220513.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/rsync-rootfs_20220515.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/fs_20220526.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/stress-ng_20220801.cgz,/osimage/pkg/debian-11.1-x86_64-20220510.cgz/stress-ng-x86_64-0.11-06_20220801.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/mpstat_20220516.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/turbostat_20220514.cgz,/osimage/pkg/debian-11.1-x86_64-20220510.cgz/turbostat-x86_64-210e04ff7681-1_20220518.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/perf_20220731.cgz,/osimage/pkg/debian-11.1-x86_64-20220510.cgz/perf-x86_64-6a010258447d-1_20220731.cgz,/osimage/pkg/debian-11.1-x86_64-20220510.cgz/sar-x86_64-c5bb321-1_20220518.cgz,/osimage/deps/debian-11.1-x86_64-20220510.cgz/hw_20220526.cgz'
	export ucode_initrd='/osimage/ucode/intel-ucode-20220216.cgz'
	export lkp_initrd='/osimage/user/lkp/lkp-x86_64.cgz'
	export site='inn'
	export LKP_CGI_PORT=80
	export LKP_CIFS_PORT=139
	export last_kernel='5.19.0-rc8-08637-g75648c1f8e7a'
	export schedule_notify_address=
	export kernel='/pkg/linux/x86_64-rhel-8.3/gcc-11/0568e6122574dcc1aded2979cd0245038efe22b6/vmlinuz-5.19.0-rc3-00027-g0568e6122574'
	export dequeue_time='2022-08-02 02:57:25 +0800'
	export job_initrd='/lkp/jobs/scheduled/lkp-icl-2sp1/stress-ng-filesystem-performance-1HDD-f2fs-10%-copy-file-60s-ucode=0xb000280-debian-11.1-x86_64-20220510.cgz-0568e6122574dcc1ade-20220801-25331-42q02g-3.cgz'

	[ -n "$LKP_SRC" ] ||
	export LKP_SRC=/lkp/${user:-lkp}/src
}

run_job()
{
	echo $$ > $TMP/run-job.pid

	. $LKP_SRC/lib/http.sh
	. $LKP_SRC/lib/job.sh
	. $LKP_SRC/lib/env.sh

	export_top_env

	run_setup nr_hdd=1 $LKP_SRC/setup/disk

	run_setup fs='f2fs' $LKP_SRC/setup/fs

	run_setup $LKP_SRC/setup/cpufreq_governor 'performance'

	run_monitor $LKP_SRC/monitors/wrapper kmsg
	run_monitor $LKP_SRC/monitors/no-stdout/wrapper boot-time
	run_monitor $LKP_SRC/monitors/wrapper uptime
	run_monitor $LKP_SRC/monitors/wrapper iostat
	run_monitor $LKP_SRC/monitors/wrapper heartbeat
	run_monitor $LKP_SRC/monitors/wrapper vmstat
	run_monitor $LKP_SRC/monitors/wrapper numa-numastat
	run_monitor $LKP_SRC/monitors/wrapper numa-vmstat
	run_monitor $LKP_SRC/monitors/wrapper numa-meminfo
	run_monitor $LKP_SRC/monitors/wrapper proc-vmstat
	run_monitor $LKP_SRC/monitors/wrapper proc-stat
	run_monitor $LKP_SRC/monitors/wrapper meminfo
	run_monitor $LKP_SRC/monitors/wrapper slabinfo
	run_monitor $LKP_SRC/monitors/wrapper interrupts
	run_monitor $LKP_SRC/monitors/wrapper lock_stat
	run_monitor lite_mode=1 $LKP_SRC/monitors/wrapper perf-sched
	run_monitor $LKP_SRC/monitors/wrapper softirqs
	run_monitor $LKP_SRC/monitors/one-shot/wrapper bdi_dev_mapping
	run_monitor $LKP_SRC/monitors/wrapper diskstats
	run_monitor $LKP_SRC/monitors/wrapper nfsstat
	run_monitor $LKP_SRC/monitors/wrapper cpuidle
	run_monitor $LKP_SRC/monitors/wrapper cpufreq-stats
	run_monitor $LKP_SRC/monitors/wrapper turbostat
	run_monitor $LKP_SRC/monitors/wrapper sched_debug
	run_monitor $LKP_SRC/monitors/wrapper perf-stat
	run_monitor $LKP_SRC/monitors/wrapper mpstat
	run_monitor debug_mode=0 $LKP_SRC/monitors/no-stdout/wrapper perf-profile
	run_monitor pmeter_server='lkp-nhm-dp2' pmeter_device='yokogawa-wt310' $LKP_SRC/monitors/wrapper pmeter
	run_monitor $LKP_SRC/monitors/wrapper oom-killer
	run_monitor $LKP_SRC/monitors/plain/watchdog

	run_test class='filesystem' test='copy-file' $LKP_SRC/tests/wrapper stress-ng
}

extract_stats()
{
	export stats_part_begin=
	export stats_part_end=

	env class='filesystem' test='copy-file' $LKP_SRC/stats/wrapper stress-ng
	$LKP_SRC/stats/wrapper kmsg
	$LKP_SRC/stats/wrapper boot-time
	$LKP_SRC/stats/wrapper uptime
	$LKP_SRC/stats/wrapper iostat
	$LKP_SRC/stats/wrapper vmstat
	$LKP_SRC/stats/wrapper numa-numastat
	$LKP_SRC/stats/wrapper numa-vmstat
	$LKP_SRC/stats/wrapper numa-meminfo
	$LKP_SRC/stats/wrapper proc-vmstat
	$LKP_SRC/stats/wrapper meminfo
	$LKP_SRC/stats/wrapper slabinfo
	$LKP_SRC/stats/wrapper interrupts
	$LKP_SRC/stats/wrapper lock_stat
	env lite_mode=1 $LKP_SRC/stats/wrapper perf-sched
	$LKP_SRC/stats/wrapper softirqs
	$LKP_SRC/stats/wrapper diskstats
	$LKP_SRC/stats/wrapper nfsstat
	$LKP_SRC/stats/wrapper cpuidle
	$LKP_SRC/stats/wrapper turbostat
	$LKP_SRC/stats/wrapper sched_debug
	$LKP_SRC/stats/wrapper perf-stat
	$LKP_SRC/stats/wrapper mpstat
	env debug_mode=0 $LKP_SRC/stats/wrapper perf-profile
	env pmeter_server='lkp-nhm-dp2' pmeter_device='yokogawa-wt310' $LKP_SRC/stats/wrapper pmeter

	$LKP_SRC/stats/wrapper time stress-ng.time
	$LKP_SRC/stats/wrapper dmesg
	$LKP_SRC/stats/wrapper kmsg
	$LKP_SRC/stats/wrapper last_state
	$LKP_SRC/stats/wrapper stderr
	$LKP_SRC/stats/wrapper time
}

"$@"

