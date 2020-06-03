// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Atmel Corporation
 *               Wenyou.Yang <wenyou.yang@atmel.com>
 */

#include <asm/io.h>
#include <common.h>

/**
 * pmc_read() - read content at address base + off into val
 *
 * @base: base address
 * @off: offset to read from
 * @val: where the content of base + off is stored
 *
 * @return: void
 */
void pmc_read(void __iomem *base, unsigned int off, unsigned int *val)
{
	*val = readl(base + off);
}

/**
 * pmc_write() - write content of val at address base + off
 *
 * @base: base address
 * @off: offset to write to
 * @val: content to be written at base + off
 *
 * @returns: void
 */
void pmc_write(void __iomem *base, unsigned int off, unsigned int val)
{
	writel(val, base + off);
}

/**
 * pmc_update_bits() - update content a set of bits at address base + off
 *
 * @base: base address
 * @off: offset to be updated
 * @mask: mask of bits to be updated
 * @bits: the new value to be updated
 *
 * @returns: void
 */
void pmc_update_bits(void __iomem *base, unsigned int off,
		     unsigned int mask, unsigned int bits)
{
	unsigned int tmp;

	tmp = readl(base + off);
	tmp &= ~mask;
	writel(tmp | (bits & mask), base + off);
}

/**
 * at91_clk_mux_val_to_index() - get parent index in mux table
 *
 * @table: clock mux table
 * @num_parents: clock number of parents
 * @val: clock id which mux index should be retrieved
 *
 * @return: clock index in mux table or a negative error number if failure
 */
int at91_clk_mux_val_to_index(const u32 *table, u32 num_parents, u32 val)
{
	int i;

	if (!table || !num_parents)
		return -EINVAL;

	for (i = 0; i < num_parents; i++) {
		if (table[i] == val)
			return i;
	}

	return -EINVAL;
}

/**
 * at91_clk_mux_index_to_val() - get parent ID corresponding to an entry in
 *	clockmux table
 *
 * @table: clock mux table
 * @num_parents: clock number of parents
 * @index: index in mux table which clock's ID should be retrieved
 *
 * @returns: clock ID or a negative error number if failure
 */
int at91_clk_mux_index_to_val(const u32 *table, u32 num_parents, u32 index)
{
	if (!table || !num_parents || index < 0 || index > num_parents)
		return -EINVAL;

	return table[index];
}
