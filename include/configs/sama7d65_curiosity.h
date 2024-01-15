/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA7D65 Curiosity Board.
 *
 * Copyright (C) 2023 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Ryan Wanner <ryan.wanner@microchip.com>
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */
#define CONFIG_SYS_BOOTM_LEN		SZ_32M
/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x60000000
#define CONFIG_SYS_SDRAM_SIZE		0x40000000

#ifdef CONFIG_SPL_BUILD
#error "No SPL support for this SoC"
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 + CONFIG_SYS_MALLOC_F_LEN - \
	 GENERATED_GBL_DATA_SIZE)
#endif

#ifndef CONFIG_BOOTCOMMAND
#ifdef CONFIG_SD_BOOT
/* u-boot env in sd/mmc card */

/* bootstrap + u-boot + env in sd card */
#define CONFIG_BOOTCOMMAND	"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x61000000 at91-sama7d65_curiosity.dtb; " \
				"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x62000000 zImage; " \
				"bootz 0x62000000 - 0x61000000"
#else
#define CONFIG_BOOTCOMMAND	"Place your bootcommand here"
#endif

#endif

#define CONFIG_ARP_TIMEOUT		200
#define CONFIG_NET_RETRY_COUNT		50

#endif
