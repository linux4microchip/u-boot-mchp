/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Special Function Register (SFR)
 *
 * Copyright (C) 2018 Microchip Technology Inc
 *		      Sandeep sheriker M <sandeep.sheriker@microchip.com>
 */

#ifndef __SAM9X60_SFR_H
#define __SAM9X60_SFR_H

struct atmel_sfr {
	u32 reserved1[1];
	u32 ebicsa;				 /* 0x04) EBI Chip Select Register */
	u32 reserved2[2];
	u32 ohciicr;         	/* 0x10) OHCI Interrupt Configuration Register */
	u32 ohciisr;         	/* 0x14) OHCI Interrupt Status Register */
	u32 otpc_confr0; 		/* 0x18) OTPC Configuration 0 Register (BackUp) */
	u32 otpc_confr1;		/* 0x1C) OTPC Configuration 1 Register (BackUp) */
	u32 rcxtaltrim;			/* 0x20) RC and XTAL Oscillator Trimming Register (BackUp) */
	u32 reserved3[3];
	u32 utmicktrim;     	 /* 0x30) UTMI Clock Trimming Register */
	u32 utmihstrim; 	     /* 0x34) UTMI High-Speed Trimming Register */
	u32 utmifstrim;	    	 /* 0x38) UTMI Full-Speed Trimming Register */
	u32 utmiswap;        /* 0x3C) UTMI DP/DM Pin Swapping Register */
	u32 reserved4[7];
	u32 rm0;             /* 0x5C) Read Margin0 Register */
	u32 rm1;             /* 0x60) Read Margin1 Register */
	u32 rm2;             /* 0x64) Read Margin2 Register */
	u32 rm3;             /* 0x68) Read Margin3 Register */
	u32 rm4;             /* 0x6C) Read Margin4 Register */
	u32 rm5;             /* 0x70) Read Margin5 Register */
	u32 reserved5[2];
	u32 ls;              /* 0x7C) Light Sleep Register */
	u32 reserved6[12];
	u32 cal0;            /* 0xB0) I/O Calibration 0 Register */
	u32 cal1;            /* 0xB4) I/O Calibration 1 Register */
	u32 reserved7[11];
	u32 wpmr;            /* 0xE4) Write Protection Mode Register */
	u32 reserved8[6];
	u32 spare;           /* 0x100) Spare Register */
	u32 buspare;        /* 0x104) Spare Register (BackUp) */
};

/* Register Mapping*/
#define AT91_SFR_UTMICKTRIM	0x30	/* UTMI Clock Trimming Register */

/* Bit field in DDRCFG */
#define ATMEL_SFR_DDRCFG_FDQIEN		0x00010000
#define ATMEL_SFR_DDRCFG_FDQSIEN	0x00020000

#define AT91_SFR_EBI_CS1A_SMC                (0 << 1)
#define AT91_SFR_EBI_CS1A_SDRAMC             (1 << 1)
#define AT91_SFR_EBI_CS3A_SMC                (0 << 3)
#define AT91_SFR_EBI_CS3A_SMC_SMARTMEDIA     (1 << 3)
#define AT91_SFR_EBI_DBPU_ON                 (0 << 8)
#define AT91_SFR_EBI_DBPU_OFF                (1 << 8)
#define AT91_SFR_EBI_DBPD_ON                 (0 << 9)
#define AT91_SFR_EBI_DBPD_OFF                (1 << 9)
#define AT91_SFR_EBI_VDDIOMSEL_1_8V          (0 << 16)
#define AT91_SFR_EBI_VDDIOMSEL_3_3V          (1 << 16)
#define AT91_SFR_EBI_EBI_IOSR_REDUCED        (0 << 17)
#define AT91_SFR_EBI_EBI_IOSR_NORMAL         (1 << 17)
#define AT91_SFR_NFD0_ON_D0                  (0 << 24)
#define AT91_SFR_NFD0_ON_D16                 (1 << 24)
#define AT91_SFR_MP_OFF                      (0 << 25)
#define AT91_SFR_MP_ON                       (1 << 25)

#define AT91_UTMICKTRIM_FREQ		GENMASK(1, 0)

/* Bit field in AICREDIR */
#define ATMEL_SFR_AICREDIR_NSAIC	0x00000001

#endif
