/*
 * SPI flash probing
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (C) 2013 Jagannadha Sutradharudu Teki, Xilinx Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"

/**
 * spi_flash_probe_slave() - Probe for a SPI flash device on a bus
 *
 * @flashp: Pointer to place to put flash info, which may be NULL if the
 * space should be allocated
 */
static int spi_flash_probe_slave(struct spi_flash *flash)
{
	struct spi_slave *spi = flash->spi;
	u8 mode_rx, e_rd_cmd;
	int ret;

	/* Setup spi_slave */
	if (!spi) {
		printf("SF: Failed to set up slave\n");
		return -ENODEV;
	}

	/* Convert SPI mode_rx and mode to SPI flash read commands */
	mode_rx = spi->mode_rx;
	if (mode_rx & SPI_RX_QUAD) {
		e_rd_cmd = RD_NORM | QUAD_OUTPUT_FAST;
		if (spi->mode & SPI_TX_QUAD)
			e_rd_cmd |= QUAD_IO_FAST;
	} else if (mode_rx & SPI_RX_DUAL) {
		e_rd_cmd = RD_NORM | DUAL_OUTPUT_FAST;
		if (spi->mode & SPI_TX_DUAL)
			e_rd_cmd |= DUAL_IO_FAST;
	} else if ((mode_rx & (SPI_RX_SLOW | SPI_RX_FAST)) == SPI_RX_SLOW) {
		e_rd_cmd = ARRAY_SLOW;
	} else {
		e_rd_cmd = RD_NORM;
	}

	/* Claim spi bus */
	ret = spi_claim_bus(spi);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	ret = spi_flash_scan(flash, e_rd_cmd);
	if (ret)
		goto err_read_id;

#ifdef CONFIG_SPI_FLASH_MTD
	ret = spi_flash_mtd_register(flash);
#endif

err_read_id:
	spi_release_bus(spi);
	return ret;
}

#ifndef CONFIG_DM_SPI_FLASH
static struct spi_flash *spi_flash_probe_tail(struct spi_slave *bus)
{
	struct spi_flash *flash;

	/* Allocate space if needed (not used by sf-uclass */
	flash = calloc(1, sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate spi_flash\n");
		return NULL;
	}

	flash->spi = bus;
	if (spi_flash_probe_slave(flash)) {
		spi_free_slave(bus);
		free(flash);
		return NULL;
	}

	return flash;
}

struct spi_flash *spi_flash_probe(unsigned int busnum, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *bus;

	bus = spi_setup_slave(busnum, cs, max_hz, spi_mode);
	if (!bus)
		return NULL;
	return spi_flash_probe_tail(bus);
}

#ifdef CONFIG_OF_SPI_FLASH
struct spi_flash *spi_flash_probe_fdt(const void *blob, int slave_node,
				      int spi_node)
{
	struct spi_slave *bus;

	bus = spi_setup_slave_fdt(blob, slave_node, spi_node);
	if (!bus)
		return NULL;
	return spi_flash_probe_tail(bus);
}
#endif

void spi_flash_free(struct spi_flash *flash)
{
#ifdef CONFIG_SPI_FLASH_MTD
	spi_flash_mtd_unregister();
#endif
	spi_free_slave(flash->spi);
	free(flash);
}

#else /* defined CONFIG_DM_SPI_FLASH */

static int spi_flash_std_read_reg(struct udevice *dev, u8 opcode, size_t len,
				  void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_read_reg_ops(flash, opcode, len, buf);
}

static int spi_flash_std_write_reg(struct udevice *dev, u8 opcode, size_t len,
				   const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_write_reg_ops(flash, opcode, len, buf);
}

static int spi_flash_std_read(struct udevice *dev, u32 offset, size_t len,
			      void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_read_ops(flash, offset, len, buf);
}

static int spi_flash_std_write(struct udevice *dev, u32 offset, size_t len,
			const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_write_ops(flash, offset, len, buf);
}

static int spi_flash_std_erase(struct udevice *dev, u32 offset, size_t len)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_cmd_erase_ops(flash, offset, len);
}

static int spi_flash_std_probe(struct udevice *dev)
{
	struct spi_slave *slave = dev_get_parent_priv(dev);
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);
	struct spi_flash *flash;

	flash = dev_get_uclass_priv(dev);
	flash->dev = dev;
	flash->spi = slave;
	debug("%s: slave=%p, cs=%d\n", __func__, slave, plat->cs);
	return spi_flash_probe_slave(flash);
}

static const struct dm_spi_flash_ops spi_flash_std_ops = {
	.read_reg = spi_flash_std_read_reg,
	.write_reg = spi_flash_std_write_reg,
	.read = spi_flash_std_read,
	.write = spi_flash_std_write,
	.erase = spi_flash_std_erase,
};

static const struct udevice_id spi_flash_std_ids[] = {
	{ .compatible = "spi-flash" },
	{ }
};

U_BOOT_DRIVER(spi_flash_std) = {
	.name		= "spi_flash_std",
	.id		= UCLASS_SPI_FLASH,
	.of_match	= spi_flash_std_ids,
	.probe		= spi_flash_std_probe,
	.priv_auto_alloc_size = sizeof(struct spi_flash),
	.ops		= &spi_flash_std_ops,
};

#endif /* CONFIG_DM_SPI_FLASH */
