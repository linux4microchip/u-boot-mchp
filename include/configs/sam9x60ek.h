/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2018 Microchip Technology Inc
 *
 * Configuation settings for the SAM9X60EK board.
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	24000000	/* 24 MHz crystal */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT

#define CONFIG_USART_BASE   ATMEL_BASE_DBGU
#define CONFIG_USART_ID     0/* ignored in arm */

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE

/*
 * define CONFIG_USB_EHCI_HCD to enable USB Hi-Speed (aka 2.0)
 * NB: in this case, USB 1.1 devices won't be recognized.
 */

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#define CONFIG_SYS_SDRAM_SIZE		0x10000000	/* 256 megs */

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 16 * 1024 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x26e00000

#ifdef CONFIG_SPI_BOOT
/* bootstrap + u-boot + env + linux in spi flash */
#define CONFIG_ENV_OFFSET	0x5000
#define CONFIG_ENV_SIZE		0x3000
#define CONFIG_ENV_SECT_SIZE	0x1000
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_BOOTCOMMAND	"sf probe 0; " \
				"sf read 0x22000000 0x100000 0x300000; " \
				"bootm 0x22000000"

#elif defined(CONFIG_SYS_USE_DATAFLASH)
/* bootstrap + u-boot + env + linux in data flash */
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_ENV_SECT_SIZE	0x210
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_BOOTCOMMAND	"sf probe 0; " \
				"sf read 0x22000000 0x84000 0x294000; " \
				"bootm 0x22000000"

#elif defined(CONFIG_SD_BOOT)
/* bootstrap + u-boot + env + linux in sd card */
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_BOOTCOMMAND  \
			"fatload mmc 0:1 0x21000000 at91-sam9x60ek.dtb;" \
			"fatload mmc 0:1 0x22000000 zImage;" \
			"bootz 0x22000000 - 0x21000000"
#else
/* bootstrap + u-boot + env + linux in emmc */
#define CONFIG_ENV_SIZE		0x4000
#endif

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024 + 0x1000)

/* SPL */
#define CONFIG_SPL_TEXT_BASE		0x300000
#define CONFIG_SPL_MAX_SIZE		0x6000
#define CONFIG_SPL_STACK		0x308000

#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR			0x1301
#define CONFIG_SYS_MCKR_CSS		0x1302

#ifdef CONFIG_SD_BOOT
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"

#elif CONFIG_SPI_BOOT
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8400
#endif

#endif
