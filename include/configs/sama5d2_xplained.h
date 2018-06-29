/*
 * Configuration file for the SAMA5D2 Xplained Board.
 *
 * Copyright (C) 2015 Atmel Corporation
 *		      Wenyou Yang <wenyou.yang@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include "at91-sama5_common.h"

#define CONFIG_MISC_INIT_R

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_DDRCS
#define CONFIG_SYS_SDRAM_SIZE		0x20000000

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x218000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000 /* load address */

/* SerialFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_BUS		0
#define CONFIG_SF_DEFAULT_CS		0
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

/* NAND flash */
#undef CONFIG_CMD_NAND

/* I2C */
#define AT24MAC_ADDR		0x5c
#define AT24MAC_REG		0x9a

/* LCD */
#ifdef CONFIG_ATMEL_HLCD
#define CONFIG_SYS_WHITE_ON_BLACK
#endif

#ifdef CONFIG_SYS_USE_MMC

#undef FAT_ENV_DEVICE_AND_PART
#undef CONFIG_BOOTARGS
#undef CONFIG_BOOTCOMMAND

#ifdef CONFIG_EMMC_BOOT
#define FAT_ENV_DEVICE_AND_PART	"0"
#define CONFIG_BOOTCOMMAND	"fatload mmc 0:1 0x21000000 at91-sama5d2_xplained.dtb; " \
				"fatload mmc 0:1 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"
#define CONFIG_BOOTARGS \
	"console=ttyS0,115200 earlyprintk root=/dev/mmcblk0p2 rw rootwait"
#else
/* bootstrap + u-boot + env in sd card */
#define FAT_ENV_DEVICE_AND_PART	"1"
#define CONFIG_BOOTCOMMAND	"fatload mmc 1:1 0x21000000 at91-sama5d2_xplained.dtb; " \
				"fatload mmc 1:1 0x22000000 zImage; " \
				"bootz 0x22000000 - 0x21000000"
#define CONFIG_BOOTARGS \
	"console=ttyS0,115200 earlyprintk root=/dev/mmcblk1p2 rw rootwait"

#endif

#endif

#ifdef CONFIG_QSPI_BOOT
#undef CONFIG_ENV_SPI_BUS
#undef CONFIG_ENV_SPI_CS
#undef CONFIG_ENV_OFFSET
#undef CONFIG_ENV_SIZE
#undef CONFIG_ENV_SECT_SIZE
#undef CONFIG_BOOTCOMMAND
#define CONFIG_ENV_SPI_BUS		1
#define CONFIG_ENV_SPI_CS		0
#define CONFIG_ENV_OFFSET		0xb0000
#define CONFIG_ENV_SIZE			0x10000
#define CONFIG_ENV_SECT_SIZE		0x10000
#define CONFIG_BOOTCOMMAND		"sf probe 1:0; "			\
					"sf read 0x21000000 0xc0000 0x20000; "	\
					"sf read 0x22000000 0xe0000 0x400000; "	\
					"bootz 0x22000000 - 0x21000000"
#endif

/* SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x200000
#define CONFIG_SPL_MAX_SIZE		0x10000
#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#ifdef CONFIG_SYS_USE_MMC
#define CONFIG_SPL_LDSCRIPT		arch/arm/mach-at91/armv7/u-boot-spl.lds
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#elif CONFIG_SYS_USE_SERIALFLASH
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x10000

#endif

#endif
