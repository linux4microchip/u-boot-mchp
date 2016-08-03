/*
 * Copyright (C) 2012 Atmel Corporation
 *
 * Copyright (C) 2016 Biagio Montaruli <biagio.hkr@gmail.com>
 *
 * U-Boot configuration settings for the Arietta-G25 board.
 * http://www.acmesystems.it/arietta
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ARIETTA_G25_H
#define __CONFIG_ARIETTA_G25_H

#include <asm/hardware.h>

#define CONFIG_SYS_TEXT_BASE		0x26f00000

#define CONFIG_ENV_IS_NOWHERE

/* ARM asynchronous clock */
#define CONFIG_SYS_AT91_SLOW_CLOCK	32768
#define CONFIG_SYS_AT91_MAIN_CLOCK	12000000	/* 12 MHz crystal */

#define CONFIG_CMDLINE_TAG		/* enable passing of ATAGs */
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_DISPLAY_CPUINFO

#define CONFIG_CMD_BOOTZ
#define CONFIG_OF_LIBFDT


/* general purpose I/O */
#define CONFIG_ATMEL_LEGACY		/* required until (g)pio is fixed */
#define CONFIG_AT91_GPIO

/* serial console */
#define CONFIG_ATMEL_USART
#define CONFIG_USART_BASE	ATMEL_BASE_DBGU
#define CONFIG_USART_ID		ATMEL_ID_SYS

#define CONFIG_BOOTDELAY	3

/*
 * BOOTP options
 */
#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

/* no NOR flash */
#define CONFIG_SYS_NO_FLASH
#if defined(CONFIG_CMD_IMLS)
#undef CONFIG_CMD_IMLS
#endif

/*
 * Command line configuration.
 */
#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_SF
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_USB

/*
 * define CONFIG_USB_EHCI to enable USB Hi-Speed (aka 2.0)
 * NB: in this case, USB 1.1 devices won't be recognized.
 */


/* SDRAM */
#define CONFIG_NR_DRAM_BANKS		1
#define CONFIG_SYS_SDRAM_BASE		0x20000000
#if defined(CONFIG_RAM_128MB)
#define CONFIG_SYS_SDRAM_SIZE		0x08000000	/* 128 mb */
#elif defined(CONFIG_RAM_256MB)
#define CONFIG_SYS_SDRAM_SIZE		0x10000000	/* 256 mb */
#endif

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_SDRAM_BASE + 4 * 1024 - GENERATED_GBL_DATA_SIZE)

/* DataFlash */
#ifdef CONFIG_CMD_SF
#define CONFIG_ATMEL_SPI
#define CONFIG_SF_DEFAULT_SPEED		30000000
#endif

/* MMC */
#ifdef CONFIG_CMD_MMC
#define CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_GENERIC_ATMEL_MCI
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#endif

/* FAT */
#ifdef CONFIG_CMD_FAT
#define CONFIG_FAT_WRITE
#define CONFIG_DOS_PARTITION
#endif

/* Ethernet */
#define CONFIG_MACB
#define CONFIG_RMII
#define CONFIG_NET_RETRY_COUNT		20
#define CONFIG_MACB_SEARCH_PHY

/* USB */
#ifdef CONFIG_CMD_USB
#ifdef CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_ATMEL
#define CONFIG_SYS_USB_EHCI_MAX_ROOT_PORTS	2
#else
#define CONFIG_USB_ATMEL
#define CONFIG_USB_ATMEL_CLK_SEL_UPLL
#define CONFIG_USB_OHCI_NEW
#define CONFIG_SYS_USB_OHCI_CPU_INIT
#define CONFIG_SYS_USB_OHCI_REGS_BASE		ATMEL_BASE_OHCI
#define CONFIG_SYS_USB_OHCI_SLOT_NAME		"arietta-g25"
#define CONFIG_SYS_USB_OHCI_MAX_ROOT_PORTS	3
#endif
#define CONFIG_USB_STORAGE
#endif

#define CONFIG_SYS_LOAD_ADDR		0x22000000	/* load address */

#define CONFIG_SYS_MEMTEST_START	CONFIG_SYS_SDRAM_BASE
#define CONFIG_SYS_MEMTEST_END		0x26e00000

#if defined(CONFIG_SYS_USE_MMC)
/* bootstrap + u-boot + env + linux in mmc */
#define CONFIG_ENV_SIZE		0x4000
#define CONFIG_BOOTCOMMAND \
		"setenv interface mmc; " \
		"setenv mmcdev 0; " \
		"setenv mmcpart 1; " \
		"setenv bootpart ${mmcdev}:${mmcpart}; " \
		"setenv linuximage zImage; " \
		"setenv dtbfile acme-arietta.dtb; " \
		"setenv loadaddr 0x22000000; " \
		"setenv dtbaddr 0x21000000; " \
		"setenv uenvaddr 0x21008000; " \
		"setenv envfile uEnv.txt; " \
		"mmc dev ${mmcdev}; " \
		"if mmc rescan; then " \
			"echo SD/MMC found on device ${mmcdev}; " \
			"if test -e mmc ${bootpart} ${linuximage}; then " \
				"echo Loading linux kernel image ${linuximage} from ${interface}${mmcdev} partition ${mmcpart} ...; " \
				"fatload mmc ${bootpart} ${loadaddr} ${linuximage}; " \
				"if test -e mmc ${bootpart} ${dtbfile}; then " \
					"echo Loading Arietta-G25 dtb ${dtbfile} from ${interface}${mmcdev} partition ${mmcpart} ...; " \
					"fatload mmc ${bootpart} ${dtbaddr} ${dtbfile}; " \
					"bootz ${loadaddr} - ${dtbaddr}; " \
				"else " \
					"echo WARN: Cannot load Arietta-G25 dtb !; " \
					"if test -e ${interface} ${bootpart} ${envfile}; then " \
						"echo Loading boot environment data from ${interface}${mmcdev} partition ${mmcpart} ...; " \
						"fatload ${interface} ${bootpart} ${uenvaddr} ${envfile}; " \
						"echo Importing boot environment from ${envfile}; " \
						"env import -t ${uenvaddr} ${filesize}; " \
						"if test -n ${uenvcmd}; then " \
							"echo Running uenvcmd ...;" \
							"run uenvcmd;" \
						"else " \
							"echo WARN: Boot environment commands not defined !; " \
						"fi;" \
					"else " \
						"echo WARN: Cannot load boot environment data from ${interface}${mmcdev} partition ${mmcpart} ...; " \
					"fi;" \
				"fi;" \
			"else " \
				"echo WARN: Cannot load linux kernel image !; " \
				"if test -e ${interface} ${bootpart} ${envfile}; then " \
					"echo Loading boot environment data from ${interface}${mmcdev} partition ${mmcpart} ...; " \
					"fatload ${interface} ${bootpart} ${uenvaddr} ${envfile}; " \
					"echo Importing boot environment from ${envfile}; " \
					"env import -t ${uenvaddr} ${filesize}; " \
					"if test -n ${uenvcmd}; then " \
						"echo Running uenvcmd ...;" \
						"run uenvcmd;" \
					"else " \
						"echo WARN: Boot environment commands not defined !; " \
					"fi;" \
				"else " \
					"echo WARN: Cannot load boot environment data from ${interface}${mmcdev} partition $mmcpart} ...; " \
				"fi;" \
			"fi;" \
		"fi;"

#elif defined(CONFIG_SYS_USE_SPIFLASH)
/* bootstrap + u-boot + env + linux in spi flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x5000
#define CONFIG_ENV_SIZE		0x3000
#define CONFIG_ENV_SECT_SIZE	0x1000
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_BOOTCOMMAND	"sf probe 0; " \
				"sf read 0x22000000 0x100000 0x300000; " \
				"bootm 0x22000000"

#elif defined(CONFIG_SYS_USE_DATAFLASH)
/* bootstrap + u-boot + env + linux in data flash */
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OFFSET	0x4200
#define CONFIG_ENV_SIZE		0x4200
#define CONFIG_ENV_SECT_SIZE	0x210
#define CONFIG_ENV_SPI_MAX_HZ	30000000
#define CONFIG_BOOTCOMMAND	"sf probe 0; " \
				"sf read 0x22000000 0x84000 0x294000; " \
				"bootm 0x22000000"
#endif

#ifdef CONFIG_SYS_USE_MMC

#ifdef CONFIG_RAM_128MB
#define CONFIG_BOOTARGS		"mem=128M console=ttyS0,115200 " \
							"root=/dev/mmcblk0p2 rw " \
							"rootfstype=ext4 rootwait"
#else /* ifdef CONFIG_RAM_256MB */
#define CONFIG_BOOTARGS		"mem=256M console=ttyS0,115200 " \
							"root=/dev/mmcblk0p2 rw " \
							"rootfstype=ext4 rootwait"
#endif /* #ifdef CONFIG_RAM_128MB */

#endif /* #ifdef CONFIG_SYS_USE_MMC */

#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_CBSIZE	256
#define CONFIG_SYS_MAXARGS	16
#define CONFIG_SYS_LONGHELP
#define CONFIG_CMDLINE_EDITING
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_HUSH_PARSER

/*
 * Size of malloc() pool
 */
#define CONFIG_SYS_MALLOC_LEN		(512 * 1024 + 0x1000)

/* SPL */
#define CONFIG_SPL_FRAMEWORK
#define CONFIG_SPL_TEXT_BASE		0x300000
#define CONFIG_SPL_MAX_SIZE			0x6000
#define CONFIG_SPL_STACK			0x308000

#define CONFIG_SPL_BSS_START_ADDR	0x20000000
#define CONFIG_SPL_BSS_MAX_SIZE		0x80000
#define CONFIG_SYS_SPL_MALLOC_START	0x20080000
#define CONFIG_SYS_SPL_MALLOC_SIZE	0x80000

#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_LIBGENERIC_SUPPORT
#define CONFIG_SPL_GPIO_SUPPORT
#define CONFIG_SPL_SERIAL_SUPPORT

#define CONFIG_SPL_BOARD_INIT
#define CONFIG_SYS_MONITOR_LEN		(512 << 10)

#define CONFIG_SYS_MASTER_CLOCK		132096000
#define CONFIG_SYS_AT91_PLLA		0x20c73f03
#define CONFIG_SYS_MCKR				0x1301
#define CONFIG_SYS_MCKR_CSS			0x1302

#ifdef CONFIG_SYS_USE_MMC
#define CONFIG_SPL_LDSCRIPT		arch/arm/mach-at91/arm926ejs/u-boot-spl.lds
#define CONFIG_SPL_MMC_SUPPORT
#define CONFIG_SYS_U_BOOT_MAX_SIZE_SECTORS	0x400
#define CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR 0x200
#define CONFIG_SYS_MMCSD_FS_BOOT_PARTITION	1
#define CONFIG_SPL_FS_LOAD_PAYLOAD_NAME		"u-boot.img"
#define CONFIG_SPL_FAT_SUPPORT
#define CONFIG_SPL_LIBDISK_SUPPORT

#elif CONFIG_SYS_USE_SPIFLASH
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x8400

#endif

#endif
