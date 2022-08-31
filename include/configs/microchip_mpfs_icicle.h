/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Microchip Technology Inc.
 * Padmarao Begari <padmarao.begari@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <linux/sizes.h>

#define CFG_SYS_SDRAM_BASE       0x80000000

/* Environment options */

#if defined(CONFIG_CMD_DHCP)
#define BOOT_TARGET_DEVICES_DHCP(func)	func(DHCP, dhcp, na)
#else
#define BOOT_TARGET_DEVICES_DHCP(func)
#endif

#if defined(CONFIG_CMD_MTD)
# define BOOT_TARGET_DEVICES_QSPI(func) func(QSPI, qspi, na)
#else
# define BOOT_TARGET_DEVICES_QSPI(func)
#endif

#if defined(CONFIG_CMD_MMC)
#define BOOT_TARGET_DEVICES_EMMC(func)	func(EMMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_EMMC(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=echo Trying to boot from QSPI...; "\
			"setenv scriptname boot.scr.uimg; " \
			"if mtd list; then setenv mtd_present true; " \
			"mtd read env ${scriptaddr} 0; " \
			"source ${scriptaddr}; setenv mtd_present; " \
			"fi\0 "

#define BOOTENV_DEV_EMMC(devtypeu, devtypel, instance) \
	"bootcmd_mmc=echo Trying to boot from eMMC/SD...;"\
			"setenv devnum 0; setenv mmcbootpart 1; setenv scriptname boot.scr.uimg;"\
			"if mmc rescan; then " \
			"load mmc 0:${mmcbootpart} ${scriptaddr} ${scriptname}; source ${scriptaddr}; " \
			"fi\0 "

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOTENV_DEV_NAME_EMMC(devtypeu, devtypel, instance) \
	"mmc "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_EMMC(func)\
	BOOT_TARGET_DEVICES_QSPI(func)\
	BOOT_TARGET_DEVICES_DHCP(func)

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"kernel_addr_r=0x84000000\0" \
	"fdt_addr_r=0x88000000\0" \
	"scriptaddr=0x88100000\0" \
	"pxefile_addr_r=0x88200000\0" \
	"ramdisk_addr_r=0x88300000\0" \
	BOOTENV

#endif /* __CONFIG_H */
