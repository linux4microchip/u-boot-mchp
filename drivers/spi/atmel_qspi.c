#include <common.h>
#include <dm.h>
#include <errno.h>
#include <spi.h>
#include <asm/io.h>
#include <mach/clk.h>

#include "atmel_qspi.h"

DECLARE_GLOBAL_DATA_PTR;


static int atmel_qspi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	/* This controller can only be used with SPI NOR flashes. */
	return -EINVAL;
}

static int atmel_qspi_set_speed(struct udevice *bus, uint hz)
{
	struct atmel_qspi_priv *aq = dev_get_priv(bus);
	unsigned long src_rate;
	u32 scr, scbr, mask, new_value;

	/* Compute the QSPI baudrate */
	src_rate = get_qspi_clk_rate(aq->dev_id);
	scbr = DIV_ROUND_UP(src_rate, hz);
	if (scbr > 0)
		scbr--;

	new_value = QSPI_SCR_SCBR_(scbr);
	mask = QSPI_SCR_SCBR;

	scr = qspi_readl(aq, QSPI_SCR);
	if ((scr & mask) == new_value)
		return 0;

	scr = (scr & ~mask) | new_value;
	qspi_writel(aq, QSPI_SCR, scr);

	return 0;
}

static int atmel_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct atmel_qspi_priv *aq = dev_get_priv(bus);
	u32 scr, mask, new_value;

	new_value = (QSPI_SCR_CPOL_((mode & SPI_CPOL) != 0) |
		     QSPI_SCR_CPHA_((mode & SPI_CPHA) != 0));
	mask = (QSPI_SCR_CPOL | QSPI_SCR_CPHA);

	scr = qspi_readl(aq, QSPI_SCR);
	if ((scr & mask) == new_value)
		return 0;

	scr = (scr & ~mask) | new_value;
	qspi_writel(aq, QSPI_SCR, scr);

	return 0;
}

static const struct dm_spi_ops atmel_qspi_ops = {
	.xfer		= atmel_qspi_xfer,
	.set_speed	= atmel_qspi_set_speed,
	.set_mode	= atmel_qspi_set_mode,
};

static int atmel_qspi_probe(struct udevice *dev)
{
	const struct atmel_qspi_platdata *plat = dev_get_platdata(dev);
	struct atmel_qspi_priv *aq = dev_get_priv(dev);
	u32 mr;

	aq->regbase = plat->regbase;
	aq->membase = plat->membase;
	aq->dev_id = plat->dev_id;

	/* Reset the QSPI controler */
	qspi_writel(aq, QSPI_CR, QSPI_CR_SWRST);

	/* Set the QSPI controller in Serial Memory Mode */
	mr = (QSPI_MR_NBBITS_8_BIT |
	      QSPI_MR_SMM_MEMORY |
	      QSPI_MR_CSMODE_LASTXFER);
	qspi_writel(aq, QSPI_MR, mr);

	/* Enable the QSPI controller */
	qspi_writel(aq, QSPI_CR, QSPI_CR_QSPIEN);

	return 0;
}

static int atmel_qspi_remove(struct udevice *dev)
{
	struct atmel_qspi_priv *aq = dev_get_priv(dev);

	/* Disable the QSPI controller */
	qspi_writel(aq, QSPI_CR, QSPI_CR_QSPIDIS);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int atmel_qspi_ofdata_to_platdata(struct udevice *dev)
{
	struct atmel_qspi_platdata *plat = dev_get_platdata(dev);
	const void *blob = gd->fdt_blob;
	int node = dev->of_offset;
	u32 data[4];
	int ret, seq;

	ret = fdtdec_get_int_array(blob, node, "reg", data, ARRAY_SIZE(data));
	if (ret) {
		printf("Error: Can't get base addresses (ret=%d)!\n", ret);
		return -ENODEV;
	}
	plat->regbase = (void *)data[0];
	plat->membase = (void *)data[2];

	ret = fdtdec_get_alias_seq(blob, "spi", node, &seq);
	if (ret) {
		printf("Error: Can't get device ID (ret=%d)!\n", ret);
		return -ENODEV;
	}
	plat->dev_id = (unsigned int)seq;

	return 0;
}

static const struct udevice_id atmel_qspi_ids[] = {
	{ .compatible = "atmel,sama5d2-qspi" },
	{ }
};
#endif

U_BOOT_DRIVER(atmel_qspi) = {
	.name		= "atmel_qspi",
	.id		= UCLASS_SPI,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match	= atmel_qspi_ids,
	.ofdata_to_platdata = atmel_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct atmel_qspi_platdata),
#endif
	.priv_auto_alloc_size = sizeof(struct atmel_qspi_priv),
	.ops		= &atmel_qspi_ops,
	.probe		= atmel_qspi_probe,
	.remove		= atmel_qspi_remove,
};
