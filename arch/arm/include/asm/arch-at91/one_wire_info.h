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

/* OWI sands for One Wire Information */

#define CM_REV_OFFSET	0
#define CM_REV_SIZE	5
#define DM_REV_OFFSET	5
#define DM_REV_SIZE	5
#define EK_REV_OFFSET	10
#define EK_REV_SIZE	5

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
