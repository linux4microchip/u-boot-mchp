/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
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
#include <mmc.h>
#include <asm/io.h>
#include <asm/arch/sama5d3_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <atmel_mci.h>
#include <net.h>
#include <netdev.h>

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#include <asm/arch/atmel_usba_udc.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_NAND_ATMEL
void sama5d3_xplained_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;

	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(5),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(8),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_TIMINGS_TCLR(3) | AT91_SMC_TIMINGS_TADL(10) |
	       AT91_SMC_TIMINGS_TAR(3)  | AT91_SMC_TIMINGS_TRR(4)   |
	       AT91_SMC_TIMINGS_TWB(5)  | AT91_SMC_TIMINGS_RBNSEL(3)|
	       AT91_SMC_TIMINGS_NFSEL(1), &smc->cs[3].timings);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);
}
#endif

#ifdef CONFIG_CMD_USB
static void sama5d3_xplained_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTE, 3, 0);
	at91_set_pio_output(AT91_PIO_PORTE, 4, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
static void sama5d3_xplained_mci0_hw_init(void)
{
	at91_mci_hw_init();

	at91_set_pio_output(AT91_PIO_PORTE, 2, 0);	/* MCI0 Power */
}

static void sama5d3_xplained_mci1_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTB, 19, 0);	/* MCI1 CMD */
	at91_set_a_periph(AT91_PIO_PORTB, 20, 0);	/* MCI1 DA0 */
	at91_set_a_periph(AT91_PIO_PORTB, 21, 0);	/* MCI1 DA1 */
	at91_set_a_periph(AT91_PIO_PORTB, 22, 0);	/* MCI1 DA2 */
	at91_set_a_periph(AT91_PIO_PORTB, 23, 0);	/* MCI1 DA3 */
	at91_set_a_periph(AT91_PIO_PORTB, 24, 0);	/* MCI1 CLK */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_MCI1);
}
#endif

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

	at91_seriald_hw_init();

	return 0;
}

int board_init(void)
{
	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_NAND_ATMEL
	sama5d3_xplained_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	sama5d3_xplained_usb_hw_init();
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	at91_udp_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	sama5d3_xplained_mci0_hw_init();
	sama5d3_xplained_mci1_hw_init();
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_MACB
	at91_gmac_hw_init();
	at91_macb_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);

	return 0;
}

int board_eth_init(bd_t *bis)
{
#ifdef CONFIG_MACB
	macb_eth_initialize(0, (void *)ATMEL_BASE_GMAC, 0x00);
	macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
#ifdef CONFIG_USB_GADGET_ATMEL_USBA
	usba_udc_probe(&pdata);
#ifdef CONFIG_USB_ETH_RNDIS
	usb_eth_initialize(bis);
#endif
#endif

	return 0;
}

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bis)
{
	atmel_mci_init((void *)ATMEL_BASE_MCI0);
	atmel_mci_init((void *)ATMEL_BASE_MCI1);

	return 0;
}
#endif

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs == 0;
}

void spi_cs_activate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTD, 13, 0);
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	at91_set_pio_output(AT91_PIO_PORTD, 13, 1);
}
#endif /* CONFIG_ATMEL_SPI */
