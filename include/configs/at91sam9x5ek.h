/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9X5EK board.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_AT91_LEGACY

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */
#define CONFIG_SYS_HZ		1000

#define CONFIG_ARM926EJS	1	/* This is an ARM926EJS Core	*/
#define CONFIG_AT91SAM9X5	1

#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG	1	/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	1

#define CONFIG_LOAD_ONE_WIRE_INFO	1
#ifdef CONFIG_LOAD_ONE_WIRE_INFO
#define CONFIG_REVISION_TAG	1       /* get the one-wire board information */
#define CONFIG_SERIAL_TAG	1
#endif

#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SKIP_RELOCATE_UBOOT

/*
 * Hardware drivers
 */
#define CONFIG_AT91_GPIO	1

/* DBGU */
#define CONFIG_ATMEL_USART	1
#define CONFIG_USART3		1	/* USART 3 is DBGU */
#define USART_BASE	USART3_BASE
#define AT91C_PIO_PA9         (1 << 9)	/* Pin Controlled by PA9 */
#define AT91C_PA9_DRXD        (AT91C_PIO_PA9)
#define AT91C_PIO_PA10        (1 << 10)	/* Pin Controlled by PA10 */
#define AT91C_PA10_DTXD       (AT91C_PIO_PA10)

#define USART3_CIDR	0x0040	/* 9x5 series chip id register */
#define USART3_EXDR	0x0044	/* 9x5 series chip id extension register */

/* LCD */
//#define CONFIG_LCD			1
#undef CONFIG_LCD
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP			24
#define CONFIG_LCD_LOGO			1
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO			1
#define CONFIG_LCD_INFO_BELOW_LOGO	1
#define CONFIG_SYS_WHITE_ON_BLACK	1
#define CONFIG_ATMEL_LCD		1
#define CONFIG_ATMEL_LCD_RGB565		1
#define CONFIG_SYS_CONSOLE_IS_IN_ENV	1
/* board specific(not enough SRAM) */
#define CONFIG_AT91SAM9X5_LCD_BASE	0x23D00000

/* LED */
#define CONFIG_AT91_LED
#define	CONFIG_RED_LED		AT91_PIN_PD31	/* this is the user1 led */
#define	CONFIG_GREEN_LED	AT91_PIN_PD0	/* this is the user2 led */

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE	1
#define CONFIG_BOOTP_BOOTPATH		1
#define CONFIG_BOOTP_GATEWAY		1
#define CONFIG_BOOTP_HOSTNAME		1

/*
 * Command line configuration.
 */
#include <config_cmd_default.h>
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_IMI
#undef CONFIG_CMD_IMLS
#undef CONFIG_CMD_AUTOSCRIPT
#undef CONFIG_CMD_LOADS

#define CONFIG_CMD_PING		1
#define CONFIG_CMD_DHCP		1
#define CONFIG_CMD_NAND		1
#undef CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			0x20000000
#define PHYS_SDRAM_SIZE			0x08000000	/* 128 megs */

/* DataFlash */
#ifdef CONFIG_ATMEL_SPI
#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#define CONFIG_SPI_FLASH		1
#define CONFIG_SPI_FLASH_ATMEL		1
#define CONFIG_SYS_MAX_DATAFLASH_BANKS	1
#define CONFIG_SYS_USE_DATAFLASH	1
#endif

/* no NOR flash */
#undef CONFIG_SYS_USE_NORFLASH
#define CONFIG_SYS_NO_FLASH	1

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_MAX_CHIPS		1
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_DBW_8		1
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE	(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE	(1 << 22)
#define CONFIG_SYS_NAND_ENABLE_PIN	AT91_PIN_PD4
#define CONFIG_SYS_NAND_READY_PIN	AT91_PIN_PD6
#define CONFIG_SYS_NAND_ALT_READY_PIN	AT91_PIN_PD5

/* PMECC & PMERRLOC */
#define CONFIG_SYS_NAND_PMECC_BASE	AT91_PMECC
#define CONFIG_SYS_NAND_PMERRLOC_BASE	AT91_PMERRLOC
//#define CONFIG_ATMEL_NAND_HWECC		1
//#define CONFIG_ATMEL_NAND_HW_PMECC	1
#endif

/* Ethernet */
#define CONFIG_MACB			1
#define CONFIG_RMII			1
#define CONFIG_NET_MULTI		1
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R		1

/* USB */
#undef CONFIG_USB_STORAGE

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	PHYS_SDRAM
#define CONFIG_SYS_MEMTEST_END		0x23e00000

#ifdef CONFIG_SYS_USE_DATAFLASH

/* bootstrap + u-boot + env + linux in dataflash on CS0 */
#define CONFIG_ENV_IS_IN_SPI_FLASH	1
#define CONFIG_SYS_MONITOR_BASE	(0x10000000 + 0x8400)
#define CONFIG_ENV_OFFSET	0x5000
#define CONFIG_ENV_ADDR		(0x10000000 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE		0x3000
#define CONFIG_ENV_SECT_SIZE	0x1000
#define CONFIG_BOOTCOMMAND	"sf probe 0; " \
				"sf read 0x22000000 0x42000 0x200000; " \
				"bootm 0x22000000"
#define CONFIG_BOOTARGS		"mem=128M console=ttyS0,115200 " \
				"mtdparts=atmel_nand:4M(bootstrap/uboot/kernel)ro,60M(rootfs),-(data) " \
				"root=/dev/mtdblock1 rw rootfstype=jffs2 "
#else /* CONFIG_SYS_USE_NANDFLASH */

/* bootstrap + u-boot + env + linux in nandflash */
#define CONFIG_ENV_IS_IN_NAND	1
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE		0x20000		/* 1 sector = 128 kB */
#define CONFIG_BOOTCOMMAND	"nand read.jffs2 " \
				"0x22000000 0x200000 0x200000; " \
				"bootm"
#define CONFIG_BOOTARGS		"mem=128M console=ttyS0,115200 " \
				"mtdparts=atmel_nand:" \
				"4M(bootstrap/uboot/kernel)ro," \
				"60M(rootfs),-(data) " \
				"root=/dev/mtdblock1 rw rootfstype=jffs2"

#endif

#define CONFIG_BAUDRATE		115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT	"U-Boot> "
#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_PBSIZE	(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) \
					+ 16)
#define CONFIG_SYS_LONGHELP	1
#define CONFIG_CMDLINE_EDITING	1
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN	ROUND(3 * CONFIG_ENV_SIZE + 128*1024, 0x1000)
#define CONFIG_SYS_GBL_DATA_SIZE	128	/* 128 bytes for initial data */

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
