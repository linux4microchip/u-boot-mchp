/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * Configuation settings for the AT91SAM9M10G45EK board(and AT91SAM9G45EKES).
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

#include <asm/hardware.h>

#define CONFIG_AT91_LEGACY

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK      32768
#define CONFIG_SYS_AT91_MAIN_CLOCK      12000000 /* from 12 MHz crystal */
#define CONFIG_SYS_HZ		        1000

#define CONFIG_AT91SAMA5EK
#define CONFIG_AT91FAMILY
#define CONFIG_ARCH_CPU_INIT
#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff	*/

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs	*/
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_OF_LIBFDT		/* Device Tree support */

/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO
#define CONFIG_AT91_GPIO_PULLUP	1	/* keep pullups on peripheral pins */

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART1
#ifdef CONFIG_USART1
#define CONFIG_USART_BASE		ATMEL_BASE_USART1
#define	CONFIG_USART_ID			ATMEL_ID_USART1
#else
#define CONFIG_USART_BASE		ATMEL_BASE_DBGU
#define	CONFIG_USART_ID			ATMEL_ID_DBGU
#endif

/*
 * This needs to be defined for the OHCI code to work but it is defined as
 * ATMEL_ID_UHPHS in the CPU specific header files.
 */
#define ATMEL_ID_UHP		ATMEL_ID_UHPHS

/*
 * Specify the clock enable bit in the PMC_SCER register.
 */
 #define ATMEL_PMC_UHP		AT91SAM926x_PMC_UHP

/* LCD */
#define CONFIG_LCD
#define LCD_BPP				LCD_COLOR16
#define LCD_OUTPUT_BPP                  24
#define CONFIG_LCD_LOGO
#undef LCD_TEST_PATTERN
#define CONFIG_LCD_INFO
#define CONFIG_LCD_INFO_BELOW_LOGO
#define CONFIG_SYS_WHITE_ON_BLACK
#define CONFIG_ATMEL_LCD
#define CONFIG_ATMEL_LCD_RGB565
#define CONFIG_SYS_CONSOLE_IS_IN_ENV

/* board specific(not enough SRAM) */
#define CONFIG_AT91SAMA5_LCD_BASE		0x23E00000

/* LED */
#if 0
#define CONFIG_AT91_LED
#define	CONFIG_RED_LED		AT91_PIN_PE24	/* this is the user1 led */
#define	CONFIG_GREEN_LED	AT91_PIN_PE25	/* this is the user2 led */
#endif

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

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


#ifdef CONFIG_SYS_USE_NANDFLASH
#define CONFIG_CMD_NAND
#elif CONFIG_SYS_USE_SERIALFLASH
#define CONFIG_CMD_SF
#endif

#define CONFIG_CMD_USB

/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE           ATMEL_BASE_DDRCS
#define CONFIG_SYS_SDRAM_SIZE		0x08000000

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

/* SerialFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_SPI_FLASH                1
#define CONFIG_SPI_FLASH_ATMEL          1
#endif

/* No NOR flash */
#define CONFIG_SYS_NO_FLASH

/* NAND flash */
#ifdef CONFIG_CMD_NAND
#define CONFIG_NAND_MAX_CHIPS			1
#define CONFIG_NAND_ATMEL
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define CONFIG_SYS_NAND_BASE			ATMEL_BASE_CS3
#define CONFIG_SYS_NAND_DBW_8
/* our ALE is AD21 */
#define CONFIG_SYS_NAND_MASK_ALE		(1 << 21)
/* our CLE is AD22 */
#define CONFIG_SYS_NAND_MASK_CLE		(1 << 22)
#define CONFIG_SYS_NAND_ONFI_DETECTION		1
#endif

/* Ethernet u-boot */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
/* Ethernet Hardware */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_GMACB
#define CONFIG_RGMII
#define CONFIG_NET_MULTI
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_RESET_PHY_R
#define CONFIG_MACB_SEARCH_PHY


/* MMC */
#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#undef  CONFIG_ATMEL_MCI_8BIT
#define ATMEL_BASE_MMCI			ATMEL_BASE_MCI0
#define CONFIG_CMD_FAT

/* USB */
#define CONFIG_USB_ATMEL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_DOS_PARTITION
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE	ATMEL_BASE_OHCI
#define CONFIG_SYS_USB_OHCI_SLOT_NAME	"at91sama5"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	3
#define CONFIG_USB_STORAGE

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x23e00000

#ifdef CONFIG_SYS_USE_SERIALFLASH
/* bootstrap + u-boot + env + linux in serial flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH      1
#define CONFIG_SYS_MONITOR_BASE (0x10000000 + 0x8400)
#define CONFIG_ENV_OFFSET       0x5000
#define CONFIG_ENV_ADDR         (0x10000000 + CONFIG_ENV_OFFSET)
#define CONFIG_ENV_SIZE         0x3000
#define CONFIG_ENV_SECT_SIZE    0x1000
#define CONFIG_BOOTCOMMAND      "sf probe 0; " \
                                "sf read 0x22000000 0x42000 0x250000; " \
                                "bootm 0x22000000"
#else
/* bootstrap + u-boot + env in nandflash */
#define CONFIG_ENV_IS_IN_NAND
#define CONFIG_ENV_OFFSET		0x60000
#define CONFIG_ENV_OFFSET_REDUND	0x80000
#define CONFIG_ENV_SIZE			0x20000
#define CONFIG_BOOTCOMMAND	"nand read 0x22000000 0x100000 0x300000;" \
				"bootm 0x22000000"
#endif

#define CONFIG_BOOTARGS							\
	"console=ttyS0,115200 earlyprintk "				\
	"root=/dev/mtdblock2 "						\
	"mtdparts=atmel_nand:1M(boot)ro,"				\
	"3M@1M(linux),-(root) "						\
	"rw rootfstype=jffs2"

#define CONFIG_BAUDRATE			115200
#define CONFIG_SYS_BAUDRATE_TABLE	{115200 , 19200, 38400, 57600, 9600 }

#define CONFIG_SYS_PROMPT		"U-Boot> "
#define CONFIG_SYS_CBSIZE		256
#define CONFIG_SYS_MAXARGS		16
#define CONFIG_SYS_PBSIZE		(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		ROUND(3 * 128 * 1024 + 128*1024, 0x1000)

#define CONFIG_STACKSIZE	(32*1024)	/* regular stack */

#ifdef CONFIG_USE_IRQ
#error CONFIG_USE_IRQ not supported
#endif

#endif
