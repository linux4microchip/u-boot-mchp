/*
 * Macronix SPI NOR flash support
 *
 * Copyright (C) 2016 Cyrille Pitchen <cyrille.pitchen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"


static int spi_flash_macronix_quad_enable(struct spi_flash *flash)
{
	int ret;
	u8 sr;

	ret = spi_flash_read_reg(flash, CMD_READ_STATUS, 1, &sr);
	if (ret)
		goto end;

	if (sr & STATUS_QEB_MXIC)
		goto end;

	sr |= STATUS_QEB_MXIC;
	ret = spi_flash_update_reg(flash, CMD_WRITE_STATUS, 1, &sr);
	if (ret)
		goto end;

	/* read SR and check it */
	ret = spi_flash_read_reg(flash, CMD_READ_STATUS, 1, &sr);
	if (ret)
		goto end;

	if (!(sr & STATUS_QEB_MXIC))
		ret = -EIO;

end:
	if (ret)
		printf("SF: Macronix SR Quad bit not set\n");

	return ret;
}

static int spi_flash_macronix_set_dummy_cycles(struct spi_flash *flash,
					       u8 num_dummy_cycles)
{
	u8 dc = (flash->read_cmd == CMD_READ_QUAD_IO_FAST) ? 2 : 0;
	u8 sr_cr[2], mask, value;
	int ret;

	/* Set DC bits */
	mask = MACRONIX_CR_DC;
	value = MACRONIX_CR_DC_(dc);

	/* Check current Configuration Register (CR) value */
	ret = spi_flash_read_reg(flash, MACRONIX_READ_CR, 1, &sr_cr[1]);
	if (ret)
		goto end;

	if ((sr_cr[1] & mask) == value)
		goto end;

	/* CR update is needed */
	ret = spi_flash_read_reg(flash, CMD_READ_STATUS, 1, &sr_cr[0]);
	if (ret)
		goto end;

	sr_cr[1] = (sr_cr[1] & ~mask) | value;
	ret = spi_flash_update_reg(flash, CMD_WRITE_STATUS, 2, sr_cr);
	if (ret)
		goto end;

	/* Verify update */
	ret = spi_flash_read_reg(flash, MACRONIX_READ_CR, 1, &sr_cr[1]);
	if (ret)
		goto end;

	if ((sr_cr[1] & mask) != value)
		ret = -EIO;

end:
	if (ret)
		printf("SF: failed to set the dummy cycles on Macronix memory!\n");

	return ret;
}

int spi_flash_setup_macronix(struct spi_flash *flash,
			     const struct spi_flash_params *params,
			     int best_match)
{
	u16 read_cmd = best_match ? (1 << (best_match - 1)) : ARRAY_FAST;
	u8 dummy_cycles = (read_cmd == ARRAY_SLOW) ? 0 : 8;
	int ret = 0;

	/* Configure (Fast) Read operations */
	switch (read_cmd) {
	case QUAD_CMD_FAST:
		if (flash->read_proto == SPI_FLASH_PROTO_4_4_4) {
			flash->read_cmd = CMD_READ_QUAD_IO_FAST;
			break;
		}
		/* Fall through SPI 1-1-4 */

	case QUAD_IO_FAST:
	case QUAD_OUTPUT_FAST:
		/*
		 * Use Fast Read 1-1-4 (0x6b) instead of 1-4-4 (0xeb) so we
		 * don't have to worry about entering the Continuous Read /
		 * Performance Enhance mode by mistake.
		 */
		ret = spi_flash_macronix_quad_enable(flash);
		flash->read_proto = SPI_FLASH_PROTO_1_1_4;
		flash->read_cmd = CMD_READ_QUAD_OUTPUT_FAST;
		break;

	case ARRAY_FAST:
		flash->read_proto = SPI_FLASH_PROTO_1_1_1;
		flash->read_cmd = CMD_READ_ARRAY_FAST;
		break;

	case ARRAY_SLOW:
		flash->read_proto = SPI_FLASH_PROTO_1_1_1;
		flash->read_cmd = CMD_READ_ARRAY_SLOW;
		break;

	default:
		return -EINVAL;
	}

	if (ret)
		return ret;

	/* Configure Page Program operations */
	if (flash->write_proto == SPI_FLASH_PROTO_4_4_4) {
		flash->write_cmd = CMD_PAGE_PROGRAM;
	} else if ((flash->spi->mode & SPI_TX_QUAD) &&
		   (params->flags & WR_QPP)) {
		flash->write_cmd = CMD_QUAD_PAGE_PROGRAM_MXIC;
		flash->write_proto = SPI_FLASH_PROTO_1_4_4;
	} else {
		flash->write_cmd = CMD_PAGE_PROGRAM;
		flash->write_proto = SPI_FLASH_PROTO_1_1_1;
	}

	/* Set number of dummy cycles for Fast Read operations */
	if (params->e_rd_cmd & QUAD_CMD_FAST)
		ret = spi_flash_macronix_set_dummy_cycles(flash, dummy_cycles);
	return ret ? : spi_flash_set_dummy_byte(flash, dummy_cycles);
}
