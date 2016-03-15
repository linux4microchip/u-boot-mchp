#include <common.h>
#include <errno.h>
#include <spi.h>
#include <spi_flash.h>

#include "sf_internal.h"
#include "../../spi/atmel_qspi.h"

struct atmel_qspi_command {
	union {
		struct {
			unsigned int	instruction:1;
			unsigned int	address:3;
			unsigned int	mode:1;
			unsigned int	dummy:1;
			unsigned int	data:1;
			unsigned int	reserved:25;
		}		bits;
		unsigned int	word;
	}			enable;
	unsigned char		instruction;
	unsigned char		mode;
	unsigned char		num_mode_cycles;
	unsigned char		num_dummy_cycles;
	unsigned int		address;

	size_t			buf_len;
	const void		*tx_buf;
	void			*rx_buf;

	enum spi_flash_protocol	protocol;
	u32			ifr_tfrtype;
};


static void atmel_qspi_memcpy_fromio(void *dst, unsigned long src, size_t len)
{
	u8 *d = (u8 *)dst;

	while (len--) {
		*d++ = readb(src);
		src++;
	}
}

static void atmel_qspi_memcpy_toio(unsigned long dst, const void *src,
				   size_t len)
{
	const u8 *s = (const u8 *)src;

	while (len--) {
		writeb(*s, dst);
		dst++;
		s++;
	}
}

static int atmel_qpsi_set_ifr_width(enum spi_flash_protocol proto, u32 *ifr)
{
	u32 ifr_width;

	switch (proto) {
	case SPI_FLASH_PROTO_1_1_1:
		ifr_width = QSPI_IFR_WIDTH_SINGLE_BIT_SPI;
		break;

	case SPI_FLASH_PROTO_1_1_2:
		ifr_width = QSPI_IFR_WIDTH_DUAL_OUTPUT;
		break;

	case SPI_FLASH_PROTO_1_2_2:
		ifr_width = QSPI_IFR_WIDTH_DUAL_IO;
		break;

	case SPI_FLASH_PROTO_2_2_2:
		ifr_width = QSPI_IFR_WIDTH_DUAL_CMD;
		break;

	case SPI_FLASH_PROTO_1_1_4:
		ifr_width = QSPI_IFR_WIDTH_QUAD_OUTPUT;
		break;

	case SPI_FLASH_PROTO_1_4_4:
		ifr_width = QSPI_IFR_WIDTH_QUAD_IO;
		break;

	case SPI_FLASH_PROTO_4_4_4:
		ifr_width = QSPI_IFR_WIDTH_QUAD_CMD;
		break;

	default:
		return -EINVAL;
	}

	*ifr = (*ifr & ~QSPI_IFR_WIDTH) | ifr_width;
	return 0;
}

static int atmel_qspi_send_command(struct atmel_qspi_priv *aq,
				   const struct atmel_qspi_command *cmd)
{
	unsigned int iar, icr, ifr;
	unsigned int offset;
	unsigned int imr, sr;
	unsigned long memaddr;
	int err;

	iar = 0;
	icr = 0;
	ifr = cmd->ifr_tfrtype;

	err = atmel_qpsi_set_ifr_width(cmd->protocol, &ifr);
	if (err)
		return err;

	/* Compute instruction parameters */
	if (cmd->enable.bits.instruction) {
		icr |= QSPI_ICR_INST_(cmd->instruction);
		ifr |= QSPI_IFR_INSTEN;
	}

	/* Compute address parameters. */
	switch (cmd->enable.bits.address) {
	case 4:
		ifr |= QSPI_IFR_ADDRL_32_BIT;
		//break; /* fall through the 24bit (3 byte) address case */
	case 3:
		iar = (cmd->enable.bits.data) ? 0 : cmd->address;
		ifr |= QSPI_IFR_ADDREN;
		offset = cmd->address;
		break;
	case 0:
		offset = 0;
		break;
	default:
		return -EINVAL;
	}

	/* Compute option parameters. */
	if (cmd->enable.bits.mode && cmd->num_mode_cycles) {
		unsigned int mode_cycle_bits, mode_bits;

		icr |= QSPI_ICR_OPT_(cmd->mode);
		ifr |= QSPI_IFR_OPTEN;

		switch (ifr & QSPI_IFR_WIDTH) {
		case QSPI_IFR_WIDTH_SINGLE_BIT_SPI:
		case QSPI_IFR_WIDTH_DUAL_OUTPUT:
		case QSPI_IFR_WIDTH_QUAD_OUTPUT:
			mode_cycle_bits = 1;
			break;
		case QSPI_IFR_WIDTH_DUAL_IO:
		case QSPI_IFR_WIDTH_DUAL_CMD:
			mode_cycle_bits = 2;
			break;
		case QSPI_IFR_WIDTH_QUAD_IO:
		case QSPI_IFR_WIDTH_QUAD_CMD:
			mode_cycle_bits = 4;
			break;
		default:
			return -EINVAL;
		}

		mode_bits = cmd->num_mode_cycles * mode_cycle_bits;
		switch (mode_bits) {
		case 1:
			ifr |= QSPI_IFR_OPTL_1BIT;
			break;

		case 2:
			ifr |= QSPI_IFR_OPTL_2BIT;
			break;

		case 4:
			ifr |= QSPI_IFR_OPTL_4BIT;
			break;

		case 8:
			ifr |= QSPI_IFR_OPTL_8BIT;
			break;

		default:
			return -EINVAL;
		}
	}

	/* Set the number of dummy cycles. */
	if (cmd->enable.bits.dummy)
		ifr |= QSPI_IFR_NBDUM_(cmd->num_dummy_cycles);

	/* Set data enable. */
	if (cmd->enable.bits.data) {
		ifr |= QSPI_IFR_DATAEN;

		/* Special case for Continuous Read Mode. */
		if (!cmd->tx_buf && !cmd->rx_buf)
			ifr |= QSPI_IFR_CRM;
	}

	/* Clear pending interrupts. */
	(void)qspi_readl(aq, QSPI_SR);

	/* Set QSPI Instruction Frame registers. */
	qspi_writel(aq, QSPI_IAR, iar);
	qspi_writel(aq, QSPI_ICR, icr);
	qspi_writel(aq, QSPI_IFR, ifr);

	/* Skip to the final steps if there is no data. */
	if (!cmd->enable.bits.data)
		goto no_data;

	/* Dummy read of QSPI_IFR to synchronize APB and AHB accesses. */
	(void)qspi_readl(aq, QSPI_IFR);

	/* Stop here for Continuous Read. */
	memaddr = (unsigned long)(aq->membase + offset);
	if (cmd->tx_buf)
		/* Write data. */
		atmel_qspi_memcpy_toio(memaddr, cmd->tx_buf, cmd->buf_len);
	else if (cmd->rx_buf)
		/* Read data. */
		atmel_qspi_memcpy_fromio(cmd->rx_buf, memaddr, cmd->buf_len);
	else
		/* Stop here for continuous read */
		return 0;

	/* Release the chip-select. */
	qspi_writel(aq, QSPI_CR, QSPI_CR_LASTXFER);

no_data:
	/* Poll INSTruction End and Chip Select Rise flags. */
	imr = (QSPI_SR_INSTRE | QSPI_SR_CSR);
	sr = 0;
	while (sr != (QSPI_SR_INSTRE | QSPI_SR_CSR))
		sr |= qspi_readl(aq, QSPI_SR) & imr;

	return 0;
}

static int atmel_qspi_read_reg(struct udevice *dev, u8 opcode,
			       size_t len, void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct atmel_qspi_priv *aq = dev_get_priv(dev->parent);
	struct atmel_qspi_command cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.enable.bits.instruction = 1;
	cmd.enable.bits.data = 1;
	cmd.instruction = opcode;
	cmd.rx_buf = buf;
	cmd.buf_len = len;
	cmd.protocol = flash->reg_proto;
	cmd.ifr_tfrtype = QSPI_IFR_TFRTYPE_READ;
	return atmel_qspi_send_command(aq, &cmd);
}

static int atmel_qspi_write_reg(struct udevice *dev, u8 opcode,
				size_t len, const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct atmel_qspi_priv *aq = dev_get_priv(dev->parent);
	struct atmel_qspi_command cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.enable.bits.instruction = 1;
	cmd.enable.bits.data = (buf && len);
	cmd.instruction = opcode;
	cmd.tx_buf = buf;
	cmd.buf_len = len;
	cmd.protocol = flash->reg_proto;
	cmd.ifr_tfrtype = QSPI_IFR_TFRTYPE_WRITE;
	return atmel_qspi_send_command(aq, &cmd);
}

static int atmel_qspi_read_impl(struct spi_flash *flash, u32 offset,
				size_t len, void *buf)
{
	struct atmel_qspi_priv *aq = dev_get_priv(flash->dev->parent);
	struct atmel_qspi_command cmd;
	u8 lshift;

	switch (SPI_FLASH_PROTO_ADR_FROM_PROTO(flash->read_proto)) {
	case 1:
		lshift = 3;
		break;

	case 2:
		lshift = 2;
		break;

	case 4:
		lshift = 1;
		break;

	default:
		return -EINVAL;
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.enable.bits.instruction = 1;
	cmd.enable.bits.address = flash->addr_width;
	cmd.enable.bits.mode = 0;
	cmd.enable.bits.dummy = (flash->dummy_byte > 0);
	cmd.enable.bits.data = 1;
	cmd.instruction = flash->read_cmd;
	cmd.address = offset;
	cmd.num_dummy_cycles = flash->dummy_byte << lshift;
	cmd.rx_buf = buf;
	cmd.buf_len = len;
	cmd.protocol = flash->read_proto;
	cmd.ifr_tfrtype = QSPI_IFR_TFRTYPE_READ_MEMORY;
	return atmel_qspi_send_command(aq, &cmd);
}

static int atmel_qspi_read(struct udevice *dev, u32 offset,
			   size_t len, void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_read_alg(flash, offset, len, buf,
				  atmel_qspi_read_impl);
}

static int atmel_qspi_write_impl(struct spi_flash *flash, u32 offset,
				 size_t len, const void *buf)
{
	struct atmel_qspi_priv *aq = dev_get_priv(flash->dev->parent);
	struct atmel_qspi_command cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.enable.bits.instruction = 1;
	cmd.enable.bits.address = flash->addr_width;
	cmd.enable.bits.data = 1;
	cmd.instruction = flash->write_cmd;
	cmd.address = offset;
	cmd.tx_buf = buf;
	cmd.buf_len = len;
	cmd.protocol = flash->write_proto;
	cmd.ifr_tfrtype = QSPI_IFR_TFRTYPE_WRITE_MEMORY;
	return atmel_qspi_send_command(aq, &cmd);
}

static int atmel_qspi_write(struct udevice *dev, u32 offset,
			    size_t len, const void *buf)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_write_alg(flash, offset, len, buf,
				   atmel_qspi_write_impl);
}

static int atmel_qspi_erase_impl(struct spi_flash *flash, u32 offset)
{
	struct atmel_qspi_priv *aq = dev_get_priv(flash->dev->parent);
	struct atmel_qspi_command cmd;

	memset(&cmd, 0, sizeof(cmd));
	cmd.enable.bits.instruction = 1;
	cmd.enable.bits.address = flash->addr_width;
	cmd.instruction = flash->erase_cmd;
	cmd.address = offset;
	cmd.protocol = flash->erase_proto;
	cmd.ifr_tfrtype = QSPI_IFR_TFRTYPE_WRITE;
	return atmel_qspi_send_command(aq, &cmd);
}

static int atmel_qspi_erase(struct udevice *dev, u32 offset, size_t len)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);

	return spi_flash_erase_alg(flash, offset, len,
				   atmel_qspi_erase_impl);
}

static const struct dm_spi_flash_ops atmel_qspi_flash_ops = {
	.read_reg	= atmel_qspi_read_reg,
	.write_reg	= atmel_qspi_write_reg,
	.read		= atmel_qspi_read,
	.write		= atmel_qspi_write,
	.erase		= atmel_qspi_erase,
};

static int atmel_qspi_flash_probe(struct udevice *dev)
{
	struct spi_flash *flash = dev_get_uclass_priv(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);
	int ret;

	flash->dev = dev;
	flash->spi = slave;

	/* Claim spi bus */
	ret = spi_claim_bus(slave);
	if (ret) {
		debug("SF: Failed to claim SPI bus: %d\n", ret);
		return ret;
	}

	/* The Quad SPI controller supports all Dual & Quad I/O protocols */
	slave->mode |= (SPI_TX_QUAD | SPI_TX_DUAL);
	ret = spi_flash_scan(flash, RD_FCMD);
	if (ret) {
		ret = -EINVAL;
		goto release;
	}

#ifdef CONFIG_SPI_FLASH_MTD
	ret = spi_flash_mtd_register(flash);
#endif

release:
	spi_release_bus(slave);
	return ret;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id atmel_qspi_flash_ids[] = {
	{ .compatible = "atmel,sama5d2-qspi-flash" },
	{ }
};
#endif

U_BOOT_DRIVER(atmel_qspi_flash) = {
	.name		= "atmel_qspi_flash",
	.id		= UCLASS_SPI_FLASH,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match	= atmel_qspi_flash_ids,
#endif
	.probe		= atmel_qspi_flash_probe,
	.ops		= &atmel_qspi_flash_ops,
};
