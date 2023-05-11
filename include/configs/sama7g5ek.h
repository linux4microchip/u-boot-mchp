/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration file for the SAMA7G5EK Board.
 *
 * Copyright (C) 2020 Microchip Corporation
 *		      Eugen Hristev <eugen.hristev@microchip.com>
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      24000000 /* from 24 MHz crystal */
#define CONFIG_SYS_BOOTM_LEN		SZ_32M

#ifdef CONFIG_SPL_BUILD
#define CONFIG_SYS_INIT_SP_ADDR		0x218000
#else
#define CONFIG_SYS_INIT_SP_ADDR \
	(0x62000000 + 16 * 1024 + CONFIG_SYS_MALLOC_F_LEN - \
	 GENERATED_GBL_DATA_SIZE)
#endif

#ifndef CONFIG_BOOTCOMMAND
#ifdef CONFIG_SD_BOOT
/* u-boot env in sd/mmc card */

/* bootstrap + u-boot + env in sd card */
#define CONFIG_BOOTCOMMAND	"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x62000000 at91-sama7g5ek.dtb; " \
				"fatload mmc " CONFIG_ENV_FAT_DEVICE_AND_PART " 0x63000000 zImage; " \
				"bootz 0x63000000 - 0x62000000"
#else
#define CONFIG_BOOTCOMMAND	"Place your bootcommand here"
#endif

#endif

#define CONFIG_ARP_TIMEOUT		200
#define CONFIG_NET_RETRY_COUNT		50

#endif
