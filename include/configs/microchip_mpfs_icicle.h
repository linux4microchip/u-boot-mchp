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
#define BOOT_TARGET_DEVICES_MMC(func)	func(MMC, mmc, 0)
#else
#define BOOT_TARGET_DEVICES_MMC(func)
#endif

#define BOOTENV_DEV_QSPI(devtypeu, devtypel, instance) \
	"bootcmd_qspi=echo Trying to boot from QSPI...; "\
			"setenv scriptname boot.scr.uimg; " \
			"if mtd list; then setenv mtd_present true; " \
			"mtd read env ${scriptaddr} 0; " \
			"source ${scriptaddr}; setenv mtd_present; " \
			"fi\0 "

#define BOOTENV_DEV_NAME_QSPI(devtypeu, devtypel, instance) \
	"qspi "

#define BOOT_TARGET_DEVICES(func) \
	BOOT_TARGET_DEVICES_QSPI(func)\
	BOOT_TARGET_DEVICES_MMC(func)\
	BOOT_TARGET_DEVICES_DHCP(func)

#define BOOTENV_DESIGN_OVERLAYS \
	"design_overlays=" \
	"if test -n ${no_of_overlays}; then " \
		"setenv inc 1; " \
		"setenv idx 0; " \
		"fdt resize ${dtbo_size}; " \
		"while test $idx -ne ${no_of_overlays}; do " \
			"setenv dtbo_name dtbo_image${idx}; " \
			"setenv fdt_cmd \"fdt apply $\"$dtbo_name; " \
			"run fdt_cmd; " \
			"setexpr idx $inc + $idx; " \
		"done; " \
	"fi;\0 " \

#include <config_distro_bootcmd.h>

#define CFG_EXTRA_ENV_SETTINGS \
	"bootm_size=0x10000000\0" \
	"scriptaddr=0x8e000000\0" \
	BOOTENV_DESIGN_OVERLAYS \
	BOOTENV \

#endif /* __CONFIG_H */
