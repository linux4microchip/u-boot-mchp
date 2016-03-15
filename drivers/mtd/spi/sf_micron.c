/*
 * Micron SPI NOR flash support
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

static int spi_flash_micron_set_dummy_cycles(struct spi_flash *flash,
					     u8 num_dummy_cycles)
{
	u8 vcr, mask, value;
	int ret;

	/* Set the number of dummy cycles and disable XIP */
	mask = (MICRON_VCR_DUMMIES | MICRON_VCR_XIP);
	value = MICRON_VCR_DUMMIES_(num_dummy_cycles) | MICRON_VCR_XIP;

	/* Check current VCR value */
	ret = spi_flash_read_reg(flash, CMD_MICRON_RD_VCR, 1, &vcr);
	if (ret)
		goto end;

	if ((vcr & mask) == value)
		goto end;

	/* VCR update is needed */
	vcr = (vcr & ~mask) | value;
	ret = spi_flash_update_reg(flash, CMD_MICRON_WR_VCR, 1, &vcr);
	if (ret)
		goto end;

	/* Verify update */
	ret = spi_flash_read_reg(flash, CMD_MICRON_RD_VCR, 1, &vcr);
	if (ret)
		goto end;

	if ((vcr & mask) != value)
		ret = -EIO;

end:
	if (ret)
		printf("SF: failed to set the dummy cycles on Micron memory!\n");

	return ret;
}

static int spi_flash_micron_set_protocol(struct spi_flash *flash,
					 enum spi_flash_protocol proto,
					 u8 mask, u8 value)
{
	u8 evcr;
	int ret;

	/* Check current EVCR value */
	ret = spi_flash_read_reg(flash, CMD_MICRON_RD_EVCR, 1, &evcr);
	if (ret)
		goto end;

	if ((evcr & mask) == value)
		goto end;

	/* EVCR update is needed */
	ret = spi_flash_cmd_write_enable(flash);
	if (ret)
		goto end;

	evcr = (evcr & ~mask) | value;
	ret = spi_flash_write_reg(flash, CMD_MICRON_WR_EVCR, 1, &evcr);
	if (ret)
		goto end;

	/*
	 * Don't forget to update the protocol HERE otherwise next commands
	 * will fail.
	 */
	flash->reg_proto = proto;

	/* Wait for ready status */
	ret = spi_flash_wait_ready(flash);
	if (ret)
		goto end;

	/* Verify update */
	ret = spi_flash_read_reg(flash, CMD_MICRON_RD_EVCR, 1, &evcr);
	if (ret)
		goto end;

	if ((evcr & mask) != value)
		ret = -EIO;

end:
	if (ret) {
		printf("SF: failed to set I/O mode on Micron memory!\n");
		return ret;
	}

	flash->read_proto = proto;
	flash->write_proto = proto;
	flash->erase_proto = proto;
	return 0;
}

static inline int spi_flash_micron_set_ext_spi_mode(struct spi_flash *flash)
{
	enum spi_flash_protocol proto = SPI_FLASH_PROTO_1_1_1;
	u8 mask = (MICRON_EVCR_QUAD_DIS | MICRON_EVCR_DUAL_DIS);
	u8 value = mask;

	return spi_flash_micron_set_protocol(flash, proto, mask, value);
}

static inline int spi_flash_micron_set_dual_mode(struct spi_flash *flash)
{
	enum spi_flash_protocol proto = SPI_FLASH_PROTO_2_2_2;
	u8 mask = (MICRON_EVCR_QUAD_DIS | MICRON_EVCR_DUAL_DIS);
	u8 value = MICRON_EVCR_QUAD_DIS;

	return spi_flash_micron_set_protocol(flash, proto, mask, value);
}

static inline int spi_flash_micron_set_quad_mode(struct spi_flash *flash)
{
	enum spi_flash_protocol proto = SPI_FLASH_PROTO_4_4_4;
	u8 mask = MICRON_EVCR_QUAD_DIS;
	u8 value = 0;

	return spi_flash_micron_set_protocol(flash, proto, mask, value);
}

int spi_flash_setup_micron(struct spi_flash *flash,
			   const struct spi_flash_params *params,
			   int best_match)
{
	u16 read_cmd = best_match ? (1 << (best_match - 1)) : ARRAY_FAST;
	u8 dummy_cycles = (read_cmd == ARRAY_SLOW) ? 0 : 8;
	int ret = 0;

	/* Configure (Fast) Read operations */
	switch (read_cmd) {
	case QUAD_CMD_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_4_4_4)
			ret = spi_flash_micron_set_quad_mode(flash);
		flash->read_cmd = CMD_READ_QUAD_IO_FAST;
		break;

	case QUAD_IO_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_1_1_1)
			ret = spi_flash_micron_set_ext_spi_mode(flash);
		flash->read_proto = SPI_FLASH_PROTO_1_4_4;
		flash->read_cmd = CMD_READ_QUAD_IO_FAST;
		break;

	case QUAD_OUTPUT_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_1_1_1)
			ret = spi_flash_micron_set_ext_spi_mode(flash);
		flash->read_proto = SPI_FLASH_PROTO_1_1_4;
		flash->read_cmd = CMD_READ_QUAD_OUTPUT_FAST;
		break;

	case DUAL_CMD_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_2_2_2)
			ret = spi_flash_micron_set_dual_mode(flash);
		flash->read_cmd = CMD_READ_DUAL_IO_FAST;
		break;

	case DUAL_IO_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_1_1_1)
			ret = spi_flash_micron_set_ext_spi_mode(flash);
		flash->read_proto = SPI_FLASH_PROTO_1_2_2;
		flash->read_cmd = CMD_READ_DUAL_IO_FAST;
		break;

	case DUAL_OUTPUT_FAST:
		if (flash->reg_proto != SPI_FLASH_PROTO_1_1_1)
			ret = spi_flash_micron_set_ext_spi_mode(flash);
		flash->read_proto = SPI_FLASH_PROTO_1_1_2;
		flash->read_cmd = CMD_READ_DUAL_OUTPUT_FAST;
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
	if (flash->write_proto == SPI_FLASH_PROTO_4_4_4 ||
	    flash->write_proto == SPI_FLASH_PROTO_2_2_2) {
		flash->write_cmd = CMD_PAGE_PROGRAM;
	} else if ((flash->spi->mode & SPI_TX_QUAD) &&
		   (params->flags & WR_QPP)) {
		flash->write_cmd = CMD_QUAD_PAGE_PROGRAM;
		flash->write_proto = SPI_FLASH_PROTO_1_1_4;
	} else {
		flash->write_cmd = CMD_PAGE_PROGRAM;
		flash->write_proto = SPI_FLASH_PROTO_1_1_1;
	}

	/* Set number of dummy cycles for Fast Read operations */
	if (params->e_rd_cmd & (QUAD_CMD_FAST | DUAL_CMD_FAST))
		ret = spi_flash_micron_set_dummy_cycles(flash, dummy_cycles);
	return ret ? : spi_flash_set_dummy_byte(flash, dummy_cycles);
}
