/*
 * Definition of One Wire information
 *
 * One Wire chip found on at91sam9x5 boards:
 * 	- CPU module
 * 	- EK mother board
 * 	- Display Module
 *
 *  Copyright (C) 2011 ATMEL Corporation,
 *  			Nicolas Ferre <nicolas.ferre@atmel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __ASM_ARCH_ONEWIREINFO_H
#define __ASM_ARCH_ONEWIREINFO_H

#if defined(CONFIG_LOAD_ONE_WIRE_INFO)

/* OWI sands for One Wire Information
 * The information comes form the 1-wire component on each board
 * and is encoded in ATAGs: both system_serial_low and system_rev
 * board revision encoding
 *
 *  ATAG_SN    lower 32 bits
 *     0-4   cpu_module_board_id         5 bits
 *     5-9   cpu_module_vendor_id        5 bits
 *     10-14 display_module_board_id     5 bits
 *     15-19 display_module_vendor_id    5 bits
 *     20-24 mother_board_id             5 bits
 *     25-29 mother_board_vendor_id      5 bits
 *     30-31 reserved for future use     2 bits
 *
 * rev: stands for revision code letter: the 'B' in "B1" revision code for
 *      instance coded as a increment from 'A' starting at 0x0: 0x0 means 'A',
 *      0x1 means 'B', etc.
 *
 * rev_id: stands for revision code identifier ;  it is a number: the '1' in
 *         "B1" revision code for instance: coded as a increment from '0'
 *         starting at 0x0: 0x0 means '0', 0x1 means '1', etc.)
 *
 *  ATAG_REV
 *     0-4   cpu_module_board_rev        5 bits
 *     5-9   display_module_board_rev    5 bits
 *     10-14 mother_module_board_rev     5 bits
 *     15-17 cpu_module_board_rev_id     3 bits
 *     18-20 display_module_board_rev_id 3 bits
 *     21-23 mother_module_board_rev_id  3 bits
 *     24-31 reserved for future use     8 bits
 */

#define CM_REV_OFFSET		0
#define CM_REV_SIZE		5
#define CM_REV_ID_OFFSET	15
#define CM_REV_ID_SIZE		3
#define DM_REV_OFFSET		5
#define DM_REV_SIZE		5
#define DM_REV_ID_OFFSET	18
#define DM_REV_ID_SIZE		3
#define EK_REV_OFFSET		10
#define EK_REV_SIZE		5
#define EK_REV_ID_OFFSET	21
#define EK_REV_ID_SIZE		3

/* Bit manipulation macros */
#define OWI_BIT(name) \
        (1 << name##_OFFSET)
#define OWI_BF(name,value) \
        (((value) & ((1 << name##_SIZE) - 1)) << name##_OFFSET)
#define OWI_BFEXT(name,value) \
        (((value) >> name##_OFFSET) & ((1 << name##_SIZE) - 1))
#define OWI_BFINS(name,value,old) \
        ( ((old) & ~(((1 << name##_SIZE) - 1) << name##_OFFSET)) \
          | SPI_BF(name,value))

#define cm_rev(rev)	OWI_BFEXT(CM_REV, (rev))
#define dm_rev(rev)	OWI_BFEXT(DM_REV, (rev))
#define ek_rev(rev)	OWI_BFEXT(EK_REV, (rev))

#define cm_is_revB(rev) (cm_rev(rev) == ('B' - 'A'))

#endif
#endif /* __ASM_ARCH_ONEWIREINFO_H */
