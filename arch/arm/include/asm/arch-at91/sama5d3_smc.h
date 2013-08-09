/*
 * Copyright (C) 2012 Atmel Corporation.
 *
 * Static Memory Controllers (SMC) - System peripherals registers.
 * Based on SAMA5D3 datasheet.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef SAMA5D3_SMC_H
#define SAMA5D3_SMC_H

#ifdef __ASSEMBLY__

#define AT91_ASM_SMC_SETUP0	(ATMEL_BASE_SMC + 0x600)
#define AT91_ASM_SMC_PULSE0	(ATMEL_BASE_SMC + 0x604)
#define AT91_ASM_SMC_CYCLE0	(ATMEL_BASE_SMC + 0x608)
#define AT91_ASM_SMC_MODE0	(ATMEL_BASE_SMC + 0x60C)

#else

typedef struct	at91_cs {
	u32	setup;		/* 0x600 SMC Setup Register */
	u32	pulse;		/* 0x604 SMC Pulse Register */
	u32	cycle;		/* 0x608 SMC Cycle Register */
	u32	timings;	/* 0x60C SMC Cycle Register */
	u32	mode;		/* 0x610 SMC Mode Register */
} at91_cs_t;

typedef struct	at91_smc {
	u32	reserved[384];
	at91_cs_t	cs[4];
} at91_smc_t;

#endif /*  __ASSEMBLY__ */

#define AT91_SMC_SETUP_NWE(x)		(x & 0x3f)
#define AT91_SMC_SETUP_NCS_WR(x)	((x & 0x3f) << 8)
#define AT91_SMC_SETUP_NRD(x)		((x & 0x3f) << 16)
#define AT91_SMC_SETUP_NCS_RD(x)	((x & 0x3f) << 24)

#define AT91_SMC_PULSE_NWE(x)		(x & 0x3f)
#define AT91_SMC_PULSE_NCS_WR(x)	((x & 0x3f) << 8)
#define AT91_SMC_PULSE_NRD(x)		((x & 0x3f) << 16)
#define AT91_SMC_PULSE_NCS_RD(x)	((x & 0x3f) << 24)

#define AT91_SMC_CYCLE_NWE(x)		(x & 0x1ff)
#define AT91_SMC_CYCLE_NRD(x)		((x & 0x1ff) << 16)

#define AT91_SMC_TIMINGS_TCLR(x)	(x & 0xf)
#define AT91_SMC_TIMINGS_TADL(x)	((x & 0xf) << 4)
#define AT91_SMC_TIMINGS_TAR(x)		((x & 0xf) << 8)
#define AT91_SMC_TIMINGS_OCMS(x)	((x & 0x1) << 12)
#define AT91_SMC_TIMINGS_TRR(x)		((x & 0xf) << 16)
#define AT91_SMC_TIMINGS_TWB(x)		((x & 0xf) << 24)
#define AT91_SMC_TIMINGS_RBNSEL(x)	((x & 0xf) << 28)
#define AT91_SMC_TIMINGS_NFSEL(x)	((x & 0x1) << 31)

#define AT91_SMC_MODE_RM_NCS		0x00000000
#define AT91_SMC_MODE_RM_NRD		0x00000001
#define AT91_SMC_MODE_WM_NCS		0x00000000
#define AT91_SMC_MODE_WM_NWE		0x00000002

#define AT91_SMC_MODE_EXNW_DISABLE	0x00000000
#define AT91_SMC_MODE_EXNW_FROZEN	0x00000020
#define AT91_SMC_MODE_EXNW_READY	0x00000030

#define AT91_SMC_MODE_BAT		0x00000100
#define AT91_SMC_MODE_DBW_8		0x00000000
#define AT91_SMC_MODE_DBW_16		0x00001000
#define AT91_SMC_MODE_DBW_32		0x00002000
#define AT91_SMC_MODE_TDF_CYCLE(x)	((x & 0xf) << 16)
#define AT91_SMC_MODE_TDF		0x00100000
#define AT91_SMC_MODE_PMEN		0x01000000
#define AT91_SMC_MODE_PS_4		0x00000000
#define AT91_SMC_MODE_PS_8		0x10000000
#define AT91_SMC_MODE_PS_16		0x20000000
#define AT91_SMC_MODE_PS_32		0x30000000

#ifdef CONFIG_AT91_LEGACY

#define AT91_SMC_SETUP(n)	(AT91_SMC + 0x00 + ((n)*0x10))	/* Setup Register for CS n */
#define		AT91_SMC_NWESETUP	(0x3f << 0)			/* NWE Setup Length */
#define			AT91_SMC_NWESETUP_(x)	((x) << 0)
#define		AT91_SMC_NCS_WRSETUP	(0x3f << 8)			/* NCS Setup Length in Write Access */
#define			AT91_SMC_NCS_WRSETUP_(x)	((x) << 8)
#define		AT91_SMC_NRDSETUP	(0x3f << 16)			/* NRD Setup Length */
#define			AT91_SMC_NRDSETUP_(x)	((x) << 16)
#define		AT91_SMC_NCS_RDSETUP	(0x3f << 24)			/* NCS Setup Length in Read Access */
#define			AT91_SMC_NCS_RDSETUP_(x)	((x) << 24)

#define AT91_SMC_PULSE(n)	(AT91_SMC + 0x04 + ((n)*0x10))	/* Pulse Register for CS n */
#define		AT91_SMC_NWEPULSE	(0x7f <<  0)			/* NWE Pulse Length */
#define			AT91_SMC_NWEPULSE_(x)	((x) << 0)
#define		AT91_SMC_NCS_WRPULSE	(0x7f <<  8)			/* NCS Pulse Length in Write Access */
#define			AT91_SMC_NCS_WRPULSE_(x)((x) << 8)
#define		AT91_SMC_NRDPULSE	(0x7f << 16)			/* NRD Pulse Length */
#define			AT91_SMC_NRDPULSE_(x)	((x) << 16)
#define		AT91_SMC_NCS_RDPULSE	(0x7f << 24)			/* NCS Pulse Length in Read Access */
#define			AT91_SMC_NCS_RDPULSE_(x)((x) << 24)

#define AT91_SMC_CYCLE(n)	(AT91_SMC + 0x08 + ((n)*0x10))	/* Cycle Register for CS n */
#define		AT91_SMC_NWECYCLE	(0x1ff << 0 )			/* Total Write Cycle Length */
#define			AT91_SMC_NWECYCLE_(x)	((x) << 0)
#define		AT91_SMC_NRDCYCLE	(0x1ff << 16)			/* Total Read Cycle Length */
#define			AT91_SMC_NRDCYCLE_(x)	((x) << 16)

#define AT91_SMC_MODE(n)	(AT91_SMC + 0x0c + ((n)*0x10))	/* Mode Register for CS n */
#define		AT91_SMC_READMODE	(1 <<  0)			/* Read Mode */
#define		AT91_SMC_WRITEMODE	(1 <<  1)			/* Write Mode */
#define		AT91_SMC_EXNWMODE	(3 <<  4)			/* NWAIT Mode */
#define			AT91_SMC_EXNWMODE_DISABLE	(0 << 4)
#define			AT91_SMC_EXNWMODE_FROZEN	(2 << 4)
#define			AT91_SMC_EXNWMODE_READY		(3 << 4)
#define		AT91_SMC_BAT		(1 <<  8)			/* Byte Access Type */
#define			AT91_SMC_BAT_SELECT		(0 << 8)
#define			AT91_SMC_BAT_WRITE		(1 << 8)
#define		AT91_SMC_DBW		(3 << 12)			/* Data Bus Width */
#define			AT91_SMC_DBW_8			(0 << 12)
#define			AT91_SMC_DBW_16			(1 << 12)
#define			AT91_SMC_DBW_32			(2 << 12)
#define		AT91_SMC_TDF		(0xf << 16)			/* Data Float Time. */
#define			AT91_SMC_TDF_(x)		((x) << 16)
#define		AT91_SMC_TDFMODE	(1 << 20)			/* TDF Optimization - Enabled */
#define		AT91_SMC_PMEN		(1 << 24)			/* Page Mode Enabled */
#define		AT91_SMC_PS		(3 << 28)			/* Page Size */
#define			AT91_SMC_PS_4			(0 << 28)
#define			AT91_SMC_PS_8			(1 << 28)
#define			AT91_SMC_PS_16			(2 << 28)
#define			AT91_SMC_PS_32			(3 << 28)
#endif
#endif
