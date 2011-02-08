/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
 * Lead Tech Design <www.leadtechdesign.com>
 *
 * (C) Copyright 2006 ATMEL Rousset, Lacressonniere Nicolas
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

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/gpio.h>
#include <asm/arch/at91_pio.h>

#include <nand.h>

#ifdef CONFIG_ATMEL_NAND_HWECC

#ifndef CONFIG_ATMEL_NAND_HW_PMECC

/* Register access macros */
#define ecc_readl(add, reg)				\
	readl(AT91_BASE_SYS + add + ATMEL_ECC_##reg)
#define ecc_writel(add, reg, value)			\
	writel((value), AT91_BASE_SYS + add + ATMEL_ECC_##reg)

#else

/* Register access macros for PMECC */
#define pmecc_readl(addr, reg) \
	readl(AT91_BASE_SYS + (addr) + ATMEL_PMECC_##reg)

#define pmecc_writel(addr, reg, value) \
	writel((value), AT91_BASE_SYS + (addr) + ATMEL_PMECC_##reg)

#define pmecc_readb_ecc(addr, sector, n) \
	readb(AT91_BASE_SYS + (addr) + ATMEL_PMECC_ECCx + \
			((sector) * 0x40) + (n))

#define pmecc_readl_rem(addr, sector, n) \
	readl(AT91_BASE_SYS + (addr) + ATMEL_PMECC_REMx + \
			((sector) * 0x40) + (n))

#define pmerrloc_readl(addr, reg) \
	readl(AT91_BASE_SYS + (addr) + ATMEL_PMERRLOC_##reg)

#define pmerrloc_writel(addr, reg, value) \
	writel((value), AT91_BASE_SYS + (addr) + ATMEL_PMERRLOC_##reg)

#define pmerrloc_writel_sigma(addr, n, value) \
	writel((value), AT91_BASE_SYS + (addr) + ATMEL_PMERRLOC_SIGMAx + \
			((n) * 4))

#define pmerrloc_readl_sigma(addr, n) \
	readl(AT91_BASE_SYS + (addr) + ATMEL_PMERRLOC_SIGMAx + ((n) * 4))

#define pmerrloc_readl_el(addr, n) \
	readl(AT91_BASE_SYS + (addr) + ATMEL_PMERRLOC_ELx + ((n) * 4))

#endif

#include "atmel_nand_ecc.h"	/* Hardware ECC registers */

/* oob layout for large page size
 * bad block info is on bytes 0 and 1
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_large = {
	.eccbytes = 4,
	.eccpos = {60, 61, 62, 63},
	.oobfree = {
		{2, 58}
	},
};

/* oob layout for small page size
 * bad block info is on bytes 4 and 5
 * the bytes have to be consecutives to avoid
 * several NAND_CMD_RNDOUT during read
 */
static struct nand_ecclayout atmel_oobinfo_small = {
	.eccbytes = 4,
	.eccpos = {0, 1, 2, 3},
	.oobfree = {
		{6, 10}
	},
};

static struct nand_ecclayout atmel_oobinfo_2048 = {
	.eccbytes = 16,
	.eccpos = { 48, 49, 50, 51, 52, 53, 54, 55,
		    56, 57, 58, 59, 60, 61, 62, 63
		  },
	.oobfree = {
		{2, 46},
	},
};

#ifndef CONFIG_ATMEL_NAND_HW_PMECC
/*
 * Calculate HW ECC
 *
 * function called after a write
 *
 * mtd:        MTD block structure
 * dat:        raw data (unused)
 * ecc_code:   buffer for ECC
 */
static int atmel_nand_calculate(struct mtd_info *mtd,
		const u_char *dat, unsigned char *ecc_code)
{
	struct nand_chip *nand_chip = mtd->priv;
	unsigned int ecc_value;

	/* get the first 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR);

	ecc_code[0] = ecc_value & 0xFF;
	ecc_code[1] = (ecc_value >> 8) & 0xFF;

	/* get the last 2 ECC bytes */
	ecc_value = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, NPR) & ATMEL_ECC_NPARITY;

	ecc_code[2] = ecc_value & 0xFF;
	ecc_code[3] = (ecc_value >> 8) & 0xFF;

	return 0;
}

/*
 * HW ECC read page function
 *
 * mtd:        mtd info structure
 * chip:       nand chip info structure
 * buf:        buffer to store read data
 */
static int atmel_nand_read_page(struct mtd_info *mtd,
		struct nand_chip *chip, uint8_t *buf, int page)
{
	int eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	uint8_t *p = buf;
	uint8_t *oob = chip->oob_poi;
	uint8_t *ecc_pos;
	int stat;

	/* read the page */
	chip->read_buf(mtd, p, eccsize);

	/* move to ECC position if needed */
	if (eccpos[0] != 0) {
		/* This only works on large pages
		 * because the ECC controller waits for
		 * NAND_CMD_RNDOUTSTART after the
		 * NAND_CMD_RNDOUT.
		 * anyway, for small pages, the eccpos[0] == 0
		 */
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT,
				mtd->writesize + eccpos[0], -1);
	}

	/* the ECC controller needs to read the ECC just after the data */
	ecc_pos = oob + eccpos[0];
	chip->read_buf(mtd, ecc_pos, eccbytes);

	/* check if there's an error */
	stat = chip->ecc.correct(mtd, p, oob, NULL);

	if (stat < 0)
		mtd->ecc_stats.failed++;
	else
		mtd->ecc_stats.corrected += stat;

	/* get back to oob start (end of page) */
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);

	/* read the oob */
	chip->read_buf(mtd, oob, mtd->oobsize);

	return 0;
}

/*
 * HW ECC Correction
 *
 * function called after a read
 *
 * mtd:        MTD block structure
 * dat:        raw data read from the chip
 * read_ecc:   ECC from the chip (unused)
 * isnull:     unused
 *
 * Detect and correct a 1 bit error for a page
 */
static int atmel_nand_correct(struct mtd_info *mtd, u_char *dat,
		u_char *read_ecc, u_char *isnull)
{
	struct nand_chip *nand_chip = mtd->priv;
	unsigned int ecc_status, ecc_parity, ecc_mode;
	unsigned int ecc_word, ecc_bit;

	/* get the status from the Status Register */
	ecc_status = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, SR);

	/* if there's no error */
	if (likely(!(ecc_status & ATMEL_ECC_RECERR)))
		return 0;

	/* get error bit offset (4 bits) */
	ecc_bit = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_BITADDR;
	/* get word address (12 bits) */
	ecc_word = ecc_readl(CONFIG_SYS_NAND_ECC_BASE, PR) & ATMEL_ECC_WORDADDR;
	ecc_word >>= 4;

	/* if there are multiple errors */
	if (ecc_status & ATMEL_ECC_MULERR) {
		/* check if it is a freshly erased block
		 * (filled with 0xff) */
		if ((ecc_bit == ATMEL_ECC_BITADDR)
				&& (ecc_word == (ATMEL_ECC_WORDADDR >> 4))) {
			/* the block has just been erased, return OK */
			return 0;
		}
		/* it doesn't seems to be a freshly
		 * erased block.
		 * We can't correct so many errors */
		printk(KERN_WARNING "atmel_nand : multiple errors detected."
				" Unable to correct.\n");
		return -EIO;
	}

	/* if there's a single bit error : we can correct it */
	if (ecc_status & ATMEL_ECC_ECCERR) {
		/* there's nothing much to do here.
		 * the bit error is on the ECC itself.
		 */
		printk(KERN_WARNING "atmel_nand : one bit error on ECC code."
				" Nothing to correct\n");
		return 0;
	}

	printk(KERN_WARNING "atmel_nand : one bit error on data."
			" (word offset in the page :"
			" 0x%x bit offset : 0x%x)\n",
			ecc_word, ecc_bit);
	/* correct the error */
	if (nand_chip->options & NAND_BUSWIDTH_16) {
		/* 16 bits words */
		((unsigned short *) dat)[ecc_word] ^= (1 << ecc_bit);
	} else {
		/* 8 bits words */
		dat[ecc_word] ^= (1 << ecc_bit);
	}
	printk(KERN_WARNING "atmel_nand : error corrected\n");
	return 1;
}

/*
 * Enable HW ECC : unused on most chips
 */
static void atmel_nand_hwctl(struct mtd_info *mtd, int mode)
{
}

static void atmel_nand_hwecc_init(struct nand_chip *nand)
{
	static int chip_nr = 0;
	struct mtd_info *mtd;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.calculate = atmel_nand_calculate;
	nand->ecc.correct = atmel_nand_correct;
	nand->ecc.hwctl = atmel_nand_hwctl;
	nand->ecc.read_page = atmel_nand_read_page;
	nand->ecc.bytes = 4;

	mtd = &nand_info[chip_nr++];
	mtd->priv = nand;

	/* Detect NAND chips */
	if (nand_scan_ident(mtd, 1)) {
		printk(KERN_WARNING "NAND Flash not found !\n");
		return -ENXIO;
	}

	if (nand->ecc.mode == NAND_ECC_HW) {
		/* ECC is calculated for the whole page (1 step) */
		nand->ecc.size = mtd->writesize;

		/* set ECC page size and oob layout */
		switch (mtd->writesize) {
		case 512:
			nand->ecc.layout = &atmel_oobinfo_small;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_528);
			break;
		case 1024:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_1056);
			break;
		case 2048:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_2112);
			break;
		case 4096:
			nand->ecc.layout = &atmel_oobinfo_large;
			ecc_writel(CONFIG_SYS_NAND_ECC_BASE, MR, ATMEL_ECC_PAGESIZE_4224);
			break;
		default:
			/* page size not handled by HW ECC */
			/* switching back to soft ECC */
			nand->ecc.mode = NAND_ECC_SOFT;
			nand->ecc.calculate = NULL;
			nand->ecc.correct = NULL;
			nand->ecc.hwctl = NULL;
			nand->ecc.read_page = NULL;
			nand->ecc.postpad = 0;
			nand->ecc.prepad = 0;
			nand->ecc.bytes = 0;
			break;
		}
	}
	return;
}
#else

#define NB_ERROR_MAX	25
struct atmel_nand_host {
	void *ecc;
	void *pmerrloc_base;
	void *rom_base;

	/* defines the error correcting capability
	 * selected at encoding or decoding time
	 */
	int tt;
	/* The number of ecc bytes for one sector */
	int ecc_bytes_per_sector;
	/* degree of the remainders, GF(2**mm) */
	int mm;
	/* length of codeword, nn=2**mm -1 */
	int nn;
	/* sector number per page */
	int sector_number;
	/* sector size in bytes */
	int sector_size;
	short *alpha_to;
	short *index_of;
	short partial_syn[100];
	short si[100];
	/* Sigma table */
	short smu[NB_ERROR_MAX + 2][2 * NB_ERROR_MAX + 1];
	/** polynomal order */
	short lmu[NB_ERROR_MAX + 1];
	u8 ecc_table[42 * 8];
};

static struct atmel_nand_host pmecc_data;

static void pmecc_gen_syndrome(struct mtd_info *mtd, int32_t sector)
{
	int32_t i;
	u32 value;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;

	/* Fill odd syndromes */
	for (i = 0; i < host->tt; i++) {
		value = pmecc_readl_rem(host->ecc, sector, i / 2);
		if (i % 2 == 0)
			host->partial_syn[(2 * i) + 1] = value & 0xffff;
		else
			host->partial_syn[(2 * i) + 1] = (value & 0xffff0000)
							  >> 16;
	}
}

static uint32_t pmecc_substitute(struct mtd_info *mtd)
{
	int32_t i, j;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int16_t *si;
	int16_t *partial_syn = host->partial_syn;
	int16_t *alpha_to = host->alpha_to;
	int16_t *index_of = host->index_of;

	/* si[] is a table that holds the current syndrome value,
	 * an element of that table belongs to the field
	 */
	si = host->si;

	for (i = 1; i < 2 * NB_ERROR_MAX; i++)
		si[i] = 0;

	/* Computation 2t syndromes based on S(x) */
	/* Odd syndromes */
	for (i = 1; i <= 2 * host->tt - 1; i = i + 2) {
		si[i] = 0;
		for (j = 0; j < host->mm; j++) {
			if (partial_syn[i] & ((unsigned short)0x1 << j))
				si[i] = alpha_to[(i * j)] ^ si[i];
		}
	}
	/* Even syndrome = (Odd syndrome) ** 2 */
	for (i = 2; i <= 2 * host->tt; i = i + 2) {
		j = i / 2;
		if (si[j] == 0)
			si[i] = 0;
		else
			si[i] = alpha_to[(2 * index_of[si[j]]) % host->nn];
	}

	return 0;
}

static uint32_t pmecc_get_sigma(struct mtd_info *mtd)
{
	int32_t i, j, k;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	unsigned int dmu_0_count, tmp;
	short *lmu = host->lmu;
	short *si = host->si;
	short tt = host->tt;
	short *index_of = host->index_of;

	/* mu          */
	int mu[NB_ERROR_MAX+1];

	/* discrepancy */
	int dmu[NB_ERROR_MAX+1];

	/* delta order   */
	int delta[NB_ERROR_MAX+1];

	/* index of largest delta */
	int ro;
	int largest;
	int diff;

	dmu_0_count = 0;

	/* First Row */

	/* Mu */
	mu[0] = -1;

	/* Actually -1/2 */
	/* Sigma(x) set to 1 */
	for (i = 0; i < 2 * NB_ERROR_MAX + 1; i++)
		host->smu[0][i] = 0;

	host->smu[0][0] = 1;

	/* discrepancy set to 1 */
	dmu[0] = 1;

	/* polynom order set to 0 */
	lmu[0] = 0;

	/* delta set to -1 */
	delta[0]  = (mu[0] * 2 - lmu[0]) >> 1;

	/* Second Row */

	/* Mu */
	mu[1]  = 0;

	/* Sigma(x) set to 1 */
	for (i = 0; i < (2 * NB_ERROR_MAX + 1); i++)
		host->smu[1][i] = 0;

	host->smu[1][0] = 1;

	/* discrepancy set to S1 */
	dmu[1] = si[1];

	/* polynom order set to 0 */
	lmu[1] = 0;

	/* delta set to 0 */
	delta[1]  = (mu[1] * 2 - lmu[1]) >> 1;

	/* Init the Sigma(x) last row */
	for (i = 0; i < (2 * NB_ERROR_MAX + 1); i++)
		host->smu[tt + 1][i] = 0;

	for (i = 1; i <= tt; i++) {
		mu[i+1] = i << 1;
		/* Compute Sigma (Mu+1)             */
		/* And L(mu)                        */
		/* check if discrepancy is set to 0 */
		if (dmu[i] == 0) {
			dmu_0_count++;

			if ((tt - (lmu[i] >> 1) - 1) & 0x1)
				tmp = ((tt - (lmu[i] >> 1) - 1) / 2) + 2;
			else
				tmp = ((tt - (lmu[i] >> 1) - 1) / 2) + 1;

			if (dmu_0_count == tmp) {
				for (j = 0; j <= (lmu[i] >> 1) + 1; j++)
					host->smu[tt + 1][j] = host->smu[i][j];

				lmu[tt + 1] = lmu[i];
				return 0;
			}

			/* copy polynom */
			for (j = 0; j <= lmu[i] >> 1; j++)
				host->smu[i + 1][j] = host->smu[i][j];

			/* copy previous polynom order to the next */
			lmu[i + 1] = lmu[i];
		} else {
			ro = 0;
			largest = -1;
			/* find largest delta with dmu != 0 */
			for (j = 0; j < i; j++) {
				if (dmu[j]) {
					if (delta[j] > largest) {
						largest = delta[j];
						ro = j;
					}
				}
			}

			/* compute difference */
			diff = (mu[i] - mu[ro]);

			/* Compute degree of the new smu polynomial */
			if ((lmu[i] >> 1) > ((lmu[ro] >> 1) + diff))
				lmu[i + 1] = lmu[i];
			else
				lmu[i + 1] = ((lmu[ro] >> 1) + diff) * 2;

			/* Init smu[i+1] with 0 */
			for (k = 0; k < (2 * NB_ERROR_MAX + 1); k++)
				host->smu[i+1][k] = 0;

			/* Compute smu[i+1] */
			for (k = 0; k <= lmu[ro] >> 1; k++)
				if (host->smu[ro][k] && dmu[i]) {
					tmp = host->index_of[dmu[i]] +
					    (host->nn
					    - host->index_of[dmu[ro]]) +
					    host->index_of[host->smu[ro][k]];
					host->smu[i + 1][k + diff] =
						host->alpha_to[tmp % host->nn];
				}

			for (k = 0; k <= lmu[i]>>1; k++)
				host->smu[i+1][k] ^= host->smu[i][k];
		}

		/*************************************************/
		/*      End Compute Sigma (Mu+1)                 */
		/*      And L(mu)                                */
		/*************************************************/
		/* In either case compute delta */
		delta[i + 1]  = (mu[i + 1] * 2 - lmu[i + 1]) >> 1;

		/* Do not compute discrepancy for the last iteration */
		if (i < tt) {
			for (k = 0 ; k <= (lmu[i + 1] >> 1); k++) {
				tmp = 2 * (i - 1);
				if (k == 0)
					dmu[i + 1] = si[tmp + 3];
				else if (host->smu[i+1][k] && si[tmp + 3 - k]) {
					tmp = index_of[host->smu[i + 1][k]] +
					      index_of[si[2 * (i - 1) + 3 - k]];
					tmp %= host->nn;
					dmu[i + 1] = host->alpha_to[tmp] ^
						     dmu[i + 1];
				}
			}
		}
	}

	return 0;
}

static int32_t pmecc_err_location(struct mtd_info *mtd)
{
	uint32_t i;
	/* number of error */
	uint32_t err_nbr;
	/* number of roots */
	uint32_t roots_nbr;
	uint32_t val;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;

	/* Disable PMECC Error Location IP */
	pmerrloc_writel(host->pmerrloc_base, ELDIS, 0xffffffff);
	err_nbr = 0;

	for (i = 0; i <= host->lmu[host->tt + 1] >> 1; i++) {
		pmerrloc_writel_sigma(host->pmerrloc_base, i,
				      host->smu[host->tt + 1][i]);
		err_nbr++;
	}

	val = pmerrloc_readl(host->pmerrloc_base, ELCFG);
	val |= ((err_nbr - 1) << 16);
	pmerrloc_writel(host->pmerrloc_base, ELCFG, val);

	/* According to the sector size to set ELEN */
	pmerrloc_writel(host->pmerrloc_base, ELEN,
			host->sector_size * 8 + 13 * host->tt);

	while (!(pmerrloc_readl(host->pmerrloc_base, ELISR)
		& PMERRLOC_CALC_DONE))
		udelay(10);


	roots_nbr = (pmerrloc_readl(host->pmerrloc_base, ELISR)
		      & PMERRLOC_ERR_NUM_MASK) >> 8;

	/* Number of roots == degree of smu hence <= tt */
	if (roots_nbr == host->lmu[host->tt + 1] >> 1)
		return err_nbr - 1;

	/* Number of roots does not match the degree of smu
	 * unable to correct error */
	return -1;
}

static uint32_t pmecc_correct_data(struct mtd_info *mtd, u8 *buf,
		uint32_t extra_bytes, uint32_t err_nbr)
{
	int i = 0;
	unsigned int byte_pos, bit_pos;
	unsigned int sector_size, ecc_size;
	unsigned int tmp;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;

	sector_size = host->sector_size;
	/* Get number of ECC bytes */
	ecc_size = nand_chip->ecc.bytes;

	while (err_nbr) {
		byte_pos = (pmerrloc_readl_el(host->pmerrloc_base, i) - 1) / 8;
		bit_pos = (pmerrloc_readl_el(host->pmerrloc_base, i) - 1) % 8;
		printk(KERN_WARNING "atmel_nand : one bit error on data."
			" (data byte : %02x, in page offset : %d,"
			" bit offset : 0x%x)\n",
			*(buf + byte_pos), byte_pos, bit_pos);
		/* If error is located in the data area(not in ECC) */
		if (byte_pos < (sector_size + extra_bytes)) {
			/* If the error position is before ECC area */
			tmp = sector_size + pmecc_readl(host->ecc, SADDR);
			if (byte_pos < tmp) {
				if (*(buf + byte_pos) & (1 << bit_pos))
					*(buf + byte_pos) &=
						(0xFF ^ (1 << bit_pos));
				else
					*(buf + byte_pos) |= (1 << bit_pos);
			} else {
				if (*(buf + byte_pos + ecc_size) &
				     (1 << bit_pos))
					*(buf + byte_pos + ecc_size) &=
						(0xFF ^ (1 << bit_pos));
				else
					*(buf + byte_pos + ecc_size) |=
						(1 << bit_pos);
			}
		}
		printk(KERN_WARNING "atmel_nand : error corrected\n");
		i++;
		err_nbr--;
	}

	return 0;
}

static int32_t pmecc_correction(struct mtd_info *mtd, uint32_t pmecc_stat,
		unsigned char *buf, u8 *ecc)
{
	int i, err_nbr;
	uint32_t buf_pos;
	struct nand_chip *nand_chip = mtd->priv;
	struct atmel_nand_host *host = nand_chip->priv;
	int eccbytes = nand_chip->ecc.bytes;

	pmerrloc_writel(host->pmerrloc_base, ELCFG,
			(host->sector_size == 512) ? 0 : 1);

	i = 0;
	while (i < host->sector_number) {
		err_nbr = 0;
		if (pmecc_stat & 0x1) {
			buf_pos = (uint32_t)(buf + i * host->sector_size);

			pmecc_gen_syndrome(mtd, i);

			pmecc_substitute(mtd);

			pmecc_get_sigma(mtd);

			err_nbr = pmecc_err_location(mtd);
			if (err_nbr == -1) {
				int j;

				for (j = 0; j < eccbytes; j++) {
					if (ecc[j] != 0xff) {
						printk(KERN_WARNING "atmel_nand"
						" : multiple errors detected."
						" Unable to correct.\n");
						return 1;
					}
				}

				/* Erased page, return OK */
				return 0;
			} else
				pmecc_correct_data(mtd, buf_pos, 0, err_nbr);
		}
		i++;
		pmecc_stat >>= 1;
	}

	return 0;
}

static int atmel_nand_pmecc_read_page(struct mtd_info *mtd,
		struct nand_chip *chip, uint8_t *buf, int page)
{
	struct atmel_nand_host *host = chip->priv;
	int32_t eccsize = chip->ecc.size;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	int32_t err = 0, stat;
	uint8_t *oob = chip->oob_poi;
	uint32_t val;

	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_RST);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_DISABLE);

	val = pmecc_readl(host->ecc, CFG);
	/* Setup to read mode */
	val &= ~PMECC_CFG_WRITE_OP;
	val |= PMECC_CFG_AUTO_ENABLE;
	pmecc_writel(host->ecc, CFG, val);

	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_ENABLE);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_DATA);

	chip->read_buf(mtd, buf, eccsize);
	chip->read_buf(mtd, oob, mtd->oobsize);

	while (pmecc_readl(host->ecc, SR) & PMECC_SR_BUSY)
		udelay(1);

	stat = pmecc_readl(host->ecc, ISR);

	if (stat != 0) {
		if (pmecc_correction(mtd, stat, buf, &oob[eccpos[0]]))
			err = -1;
	}

	return err;
}

static void atmel_nand_pmecc_write_page(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf)
{
	int i, j;
	struct atmel_nand_host *host = chip->priv;
	uint32_t *eccpos = chip->ecc.layout->eccpos;
	unsigned char *pp;
	uint32_t val;

	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_RST);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_DISABLE);

	val = pmecc_readl(host->ecc, CFG);
	val |= PMECC_CFG_WRITE_OP;
	val &= ~PMECC_CFG_AUTO_ENABLE;
	pmecc_writel(host->ecc, CFG, val);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_ENABLE);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_DATA);
	chip->write_buf(mtd, buf, mtd->writesize);

	while (pmecc_readl(host->ecc, SR) & PMECC_SR_BUSY)
		udelay(1);

	for (i = 0; i < host->sector_number; i++) {
		for (j = 0; j < host->ecc_bytes_per_sector; j++) {
			int pos;

			pos = i * host->ecc_bytes_per_sector + j;
			chip->oob_poi[eccpos[pos]] =
				pmecc_readb_ecc(host->ecc, i, j);
		}
	}
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);

	return;
}

static short *pmecc_get_alpha_to(struct atmel_nand_host *host)
{
	short *p;
	if (host->sector_size == 512) {
		p = (short *)((u32)host->rom_base + 0x8000);
		return p + 0x2000;
	} else {
		p = (short *)((u32)host->rom_base + 0x10000);
		return p + 0x4000;
	}
}

static short *pmecc_get_index_of(struct atmel_nand_host *host)
{
	short *p = (short *)host->rom_base;
	if (host->sector_size == 512)
		p = (short *)((u32)host->rom_base + 0x8000);
	else
		p = (short *)((u32)host->rom_base + 0x10000);

	return p;
}

static int initialize_pmecc_core(struct mtd_info *mtd)
{
	struct nand_chip *nand = mtd->priv;
	struct atmel_nand_host *host = nand->priv;

	uint32_t val;
	struct nand_ecclayout *ecc_layout;

	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_DISABLE);
	pmecc_writel(host->ecc, CTRL, PMECC_CTRL_RST);

	val = 0;
	switch (host->tt) {
	case 2:
		val = PMECC_CFG_BCH_ERR2;
		break;
	case 4:
		val = PMECC_CFG_BCH_ERR4;
		break;
	case 8:
		val = PMECC_CFG_BCH_ERR8;
		break;
	case 12:
		val = PMECC_CFG_BCH_ERR12;
		break;
	case 24:
		val = PMECC_CFG_BCH_ERR24;
		break;
	}

	if (host->sector_size == 512)
		val |= PMECC_CFG_SECTOR512;
	else if (host->sector_size == 1024)
		val |= PMECC_CFG_SECTOR1024;

	switch (host->sector_number) {
	case 1:
		val |= PMECC_CFG_PAGE_1SECTOR;
		break;
	case 2:
		val |= PMECC_CFG_PAGE_2SECTORS;
		break;
	case 4:
		val |= PMECC_CFG_PAGE_4SECTORS;
		break;
	case 8:
		val |= PMECC_CFG_PAGE_8SECTORS;
		break;
	}

	val |= PMECC_CFG_READ_OP | PMECC_CFG_SPARE_DISABLE
		| PMECC_CFG_AUTO_DISABLE;
	pmecc_writel(host->ecc, CFG, val);

	ecc_layout = nand->ecc.layout;
	pmecc_writel(host->ecc, SAREA, mtd->oobsize - 1);
	pmecc_writel(host->ecc, SADDR, ecc_layout->eccpos[0]);
	pmecc_writel(host->ecc, EADDR,
			ecc_layout->eccpos[ecc_layout->eccbytes - 1]);
	pmecc_writel(host->ecc, CLK, PMECC_CLK_133MHZ);
	pmecc_writel(host->ecc, IDR, 0xff);

	val = pmecc_readl(host->ecc, CTRL);
	val |= PMECC_CTRL_ENABLE;
	pmecc_writel(host->ecc, CTRL, val);

	return 0;
}

static void atmel_nand_pmecc_init(struct nand_chip *nand)
{
	static int chip_nr = 0;
	struct mtd_info *mtd;
	struct atmel_nand_host *host;
	nand->priv = &pmecc_data;

	nand->ecc.mode = NAND_ECC_HW;
	nand->ecc.calculate = 0;
	nand->ecc.correct = 0;
	nand->ecc.hwctl = 0;
	nand->ecc.read_page = atmel_nand_pmecc_read_page;
	nand->ecc.write_page = atmel_nand_pmecc_write_page;

	mtd = &nand_info[chip_nr++];
	mtd->priv = nand;
	host = nand->priv;

	/* Detect NAND chips */
	if (nand_scan_ident(mtd, 1)) {
		printk(KERN_WARNING "NAND Flash not found !\n");
		return -ENXIO;
	}

	if (nand->ecc.mode == NAND_ECC_HW) {
		/* ECC is calculated for the whole page (1 step) */
		nand->ecc.size = mtd->writesize;

		/* set ECC page size and oob layout */
		switch (mtd->writesize) {
		case 512:
			/* not implement yet */
			BUG();
			break;
		case 1024:
			/* not implement yet */
			BUG();
			break;
		case 2048:
			nand->ecc.bytes = 16;
			nand->ecc.steps = 1;
			nand->ecc.layout = &atmel_oobinfo_2048;
			host->ecc = CONFIG_SYS_NAND_PMECC_BASE;
			host->pmerrloc_base = CONFIG_SYS_NAND_PMERRLOC_BASE;
			host->rom_base = AT91SAM9X5_ROM_BASE;
			host->mm = 13;
			host->nn = (1 << host->mm) - 1;
			host->tt = 2;
			host->sector_size = 512;
			host->sector_number = mtd->writesize
						/ host->sector_size;
			host->ecc_bytes_per_sector = 4;
			host->alpha_to = pmecc_get_alpha_to(host);
			host->index_of = pmecc_get_index_of(host);
			break;
		case 4096:
			/* not implement yet */
			BUG();
			break;
		default:
			/* page size not handled by HW ECC */
			/* switching back to soft ECC */
			nand->ecc.mode = NAND_ECC_SOFT;
			nand->ecc.calculate = NULL;
			nand->ecc.correct = NULL;
			nand->ecc.hwctl = NULL;
			nand->ecc.read_page = NULL;
			nand->ecc.postpad = 0;
			nand->ecc.prepad = 0;
			nand->ecc.bytes = 0;
			break;
		}

		/* pmecc hardware initialization */
		initialize_pmecc_core(mtd);
	}
	return;
}

#endif

#endif

static void at91_nand_hwcontrol(struct mtd_info *mtd,
					 int cmd, unsigned int ctrl)
{
	struct nand_chip *this = mtd->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		ulong IO_ADDR_W = (ulong) this->IO_ADDR_W;
		IO_ADDR_W &= ~(CONFIG_SYS_NAND_MASK_ALE
			     | CONFIG_SYS_NAND_MASK_CLE);

		if (ctrl & NAND_CLE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_CLE;
		if (ctrl & NAND_ALE)
			IO_ADDR_W |= CONFIG_SYS_NAND_MASK_ALE;

		at91_set_gpio_value(CONFIG_SYS_NAND_ENABLE_PIN,
				    !(ctrl & NAND_NCE));
		this->IO_ADDR_W = (void *) IO_ADDR_W;
	}

	if (cmd != NAND_CMD_NONE)
		writeb(cmd, this->IO_ADDR_W);
}

#ifdef CONFIG_SYS_NAND_ALT_READY_PIN
static int at91_nand_ready(struct mtd_info *mtd)
{
	return at91_get_gpio_value(CONFIG_SYS_NAND_ALT_READY_PIN);
}
#elif defined(CONFIG_SYS_NAND_READY_PIN)
static int at91_nand_ready(struct mtd_info *mtd)
{
	return at91_get_gpio_value(CONFIG_SYS_NAND_READY_PIN);
}
#endif

int board_nand_init(struct nand_chip *nand)
{
	nand->ecc.mode = NAND_ECC_SOFT;
#ifdef CONFIG_SYS_NAND_DBW_16
	nand->options = NAND_BUSWIDTH_16;
#endif
	nand->cmd_ctrl = at91_nand_hwcontrol;
#if defined(CONFIG_SYS_NAND_READY_PIN) || defined(CONFIG_SYS_NAND_ALT_READY_PIN)
	nand->dev_ready = at91_nand_ready;
#endif
	nand->chip_delay = 20;

#ifdef CONFIG_ATMEL_NAND_HWECC
#ifdef CONFIG_ATMEL_NAND_HW_PMECC
	atmel_nand_pmecc_init(nand);
#else
	atmel_nand_hwecc_init(nand);
#endif
#endif
	return 0;
}
