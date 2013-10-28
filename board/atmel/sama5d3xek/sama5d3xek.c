/*
 * Copyright (C) 2012 - 2013 Atmel Corporation
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
#include <lcd.h>
#include <atmel_lcdc.h>
#include <atmel_mci.h>
#if defined(CONFIG_RESET_PHY_R) && defined(CONFIG_MACB)
#include <net.h>
#endif
#include <netdev.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */

#ifdef CONFIG_CMD_NAND
void sama5d3xek_nand_hw_init(void)
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
static void sama5d3xek_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTD, 25, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 26, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 27, 0);
}
#endif

#ifdef CONFIG_GENERIC_ATMEL_MCI
static void sama5d3xek_mci_hw_init(void)
{
	at91_mci_hw_init();
}
#endif

#ifdef CONFIG_MACB
static void sama5d3xek_macb_hw_init(void)
{
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GMACB
static void sama5d3xek_gmacb_hw_init(void)
{
	at91_gmacb_hw_init();
}
#endif

#ifdef CONFIG_LCD

vidinfo_t panel_info = {
	vl_col:		800,
	vl_row:		480,
	vl_clk:		24000000,
	vl_sync:	ATMEL_LCDC_INVLINE_NORMAL |
			ATMEL_LCDC_INVFRAME_NORMAL,
	vl_bpix:	LCD_BPP,
	vl_tft:		1,
	vl_hsync_len:	128,
	vl_left_margin:	64,
	vl_right_margin:64,
	vl_vsync_len:	2,
	vl_upper_margin:22,
	vl_lower_margin:21,
	mmio :		 ATMEL_BASE_LCDC,
};


void lcd_enable(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 29, 1);	/* power up */
}

void lcd_disable(void)
{
	at91_set_a_periph(AT91_PIO_PORTA, 29, 0);	/* power down */
}

static void sama5d3xek_lcd_hw_init(void)
{
	gd->fb_base = CONFIG_SAMA5D3_LCD_BASE;

	at91_lcd_hw_init();
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size;
	uint64_t nand_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2012 ATMEL Corp\n");
	lcd_printf ("at91support@atmel.com\n");
	lcd_printf ("%s CPU at %s MHz\n",
		get_cpu_name(),
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
#ifdef CONFIG_CMD_NAND
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;
#endif
	lcd_printf ("  %ld MB SDRAM, %lld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20 );
}
#endif /* CONFIG_LCD_INFO */
#endif

int board_early_init_f(void)
{
	at91_periph_clk_enable(ATMEL_ID_PIOA);
	at91_periph_clk_enable(ATMEL_ID_PIOB);
	at91_periph_clk_enable(ATMEL_ID_PIOC);
	at91_periph_clk_enable(ATMEL_ID_PIOD);
	at91_periph_clk_enable(ATMEL_ID_PIOE);

#ifdef CONFIG_USART1
	at91_serial1_hw_init();
#else
	at91_seriald_hw_init();
#endif
	return 0;
}

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	sama5d3xek_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	sama5d3xek_usb_hw_init();
#endif
#ifdef CONFIG_GENERIC_ATMEL_MCI
	sama5d3xek_mci_hw_init();
#endif
#ifdef CONFIG_SYS_USE_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_MACB
	if(has_emac())
		sama5d3xek_macb_hw_init();
#endif
#ifdef CONFIG_GMACB
	if (has_gmac())
		sama5d3xek_gmacb_hw_init();
#endif
#ifdef CONFIG_LCD
	if (has_lcdc())
		sama5d3xek_lcd_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
				    CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	if (has_emac())
		rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
#ifdef CONFIG_GMACB
	if (has_gmac())
		rc = gmacb_eth_initialize(0, (void *)ATMEL_BASE_GMAC, 0x00);
#endif
	return rc;
}

#ifdef CONFIG_GENERIC_ATMEL_MCI
int board_mmc_init(bd_t *bis)
{
	int rc = 0;

	rc = atmel_mci_init((void *)ATMEL_BASE_MCI0);

	return rc;
}
#endif

/* SPI chip select control */
#ifdef CONFIG_ATMEL_SPI
#include <spi.h>

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
	return bus == 0 && cs < 4;
}

void spi_cs_activate(struct spi_slave *slave)
{
	switch(slave->cs) {
		case 0:
		default:
			at91_set_pio_output(AT91_PIO_PORTD, 13, 0);
			break;
	}
}

void spi_cs_deactivate(struct spi_slave *slave)
{
	switch(slave->cs) {
		case 0:
		default:
			at91_set_pio_output(AT91_PIO_PORTD, 13, 1);
		break;
	}
}
#endif /* CONFIG_ATMEL_SPI */

#ifdef CONFIG_OF_BOARD_SETUP

#include <libfdt.h>

#define GPBR_ONEWIRE_INFO	3	/* one wire information are stored in GPBR3 */
#define PINCTRL_CELL_SIZE	16	/* four 32-bit words */
#define PINCTRL_PIO_CTRL	3
#define PINCTRL_PIO_NUM		7
#define PINCTRL_PIO_PERIPH	11
#define PINCTRL_PIO_CONF	15

/*
 * SAMA5D3 GPBR3 content:
 * bit 0-4: cpu module revision code
 * bit 5-9: display module revision code
 * bit 10-14: motherboard revision code
 * bit 15-17: cpu module revision id
 * bit 18-20: display module revision code
 * bit 21-23: motherboard revision code
 */
char get_mb_rev_code(int gpbr_reg)
{
	char mb_rev_code = 'A';

	mb_rev_code += (readl(ATMEL_BASE_GPBR + 4 * gpbr_reg) >> 10) & 0x1f;

	return mb_rev_code;
}

void ft_board_setup(void *blob, bd_t *bd)
{
	int off, off_pinctrl, err;
	char mb_rev_code[2];
	char pinctrl_isi_pck_as_mck[PINCTRL_CELL_SIZE];
	char pinctrl_isi_reset[PINCTRL_CELL_SIZE];

	printf("Device tree update:\n");

	/* set atmel,mb-rev property */
	mb_rev_code[0] = get_mb_rev_code(GPBR_ONEWIRE_INFO);
	mb_rev_code[1] = '\0';
	err = fdt_setprop(blob, 0, "atmel,mb-rev", mb_rev_code, 2);
	if (err < 0) {
		printf("  error %d while setting atmel,mb-rev property\n", err);
		return;
	}
	printf("  mb-rev property set to %s\n", mb_rev_code);

	/* update ISI pinctrl */
	if (mb_rev_code[0] == 'B') {
		off_pinctrl = fdt_node_offset_by_compatible(blob, -1, "atmel,at91sam9x5-pinctrl");
		if (off_pinctrl < 0) {
			printf("  error %d while looking for pinctrl node\n", off_pinctrl);
			return;
		}
		off = fdt_subnode_offset(blob, off_pinctrl, "isi");
		if (off < 0) {
			printf("  error %d while looking for pinctrl isi subnode\n", off);
			return;
		}
		/* ISI_MCK */
		off = fdt_subnode_offset(blob, off, "isi_pck_as_mck-0");
		if (off < 0) {
			printf("  error %d while looking for pinctrl isi_pck_as_mck-0 node\n", off);
			return;
		}
		memset(pinctrl_isi_pck_as_mck, 0, sizeof(pinctrl_isi_pck_as_mck));
		pinctrl_isi_pck_as_mck[PINCTRL_PIO_CTRL] = 2;	/* pio C */
		pinctrl_isi_pck_as_mck[PINCTRL_PIO_NUM] = 15;	/* 15 */
		pinctrl_isi_pck_as_mck[PINCTRL_PIO_PERIPH] = 2;	/* periph B */
		err = fdt_setprop_inplace(blob, off, "atmel,pins", pinctrl_isi_pck_as_mck, sizeof(pinctrl_isi_pck_as_mck));
		if (err < 0) {
			printf("  error %d while updating isi_pck_as_mck-0 node\n", err);
			return;
		}
		/* ZB_SLPTR and ZB_RSTN */
		off = fdt_subnode_offset(blob, off_pinctrl, "board");
		if (off < 0) {
			printf("  error %d while looking for pinctrl board subnode\n", off);
			return;
		}
		off = fdt_subnode_offset(blob, off, "isi_reset-0");
		if (off < 0) {
			printf("  error %d while looking for pinctrl isi_reset-0 node\n", off);
			return;
		}
		memset(pinctrl_isi_reset, 0, sizeof(pinctrl_isi_reset));
		pinctrl_isi_reset[PINCTRL_PIO_CTRL] = 4;	/* pio E */
		pinctrl_isi_reset[PINCTRL_PIO_NUM] = 28;	/* 28 */
		pinctrl_isi_reset[PINCTRL_PIO_PERIPH] = 0;	/* gpio */
		err = fdt_setprop_inplace(blob, off, "atmel,pins", pinctrl_isi_reset, sizeof(pinctrl_isi_reset));
		if (err < 0) {
			printf("  error %d while updating isi_reset-0 node\n", err);
			return;
		}
		printf("  pinctrl for isi on mb rev B successfully updated\n");
	}
}

#endif /* CONFIG_OF_BOARD_SETUP */
