/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SAM9X75EB board.
 *
 * Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Balamanikandan Gunasundar <balamanikandan.gunasundar@microchip.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	24000000	/* 24 MHz crystal */

#define CONFIG_USART_BASE   ATMEL_BASE_DBGU
#define CONFIG_USART_ID     0 /* ignored in arm */

/* SDRAM */
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000      /* 256 megs */

/* Ethernet */
#define CONFIG_RESET_PHY_R

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif

#endif
