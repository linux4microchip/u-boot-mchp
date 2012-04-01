/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian@popies.net>
 * Lead Tech Design <www.leadtechdesign.com>
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
#include <asm/io.h>
#include <asm/arch/at91sama5_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>
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
void at91sama5ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	writel(1 << ATMEL_ID_SMC, &pmc->pcer);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(2) | AT91_SMC_SETUP_NCS_WR(1) |
	       AT91_SMC_SETUP_NRD(2) | AT91_SMC_SETUP_NCS_RD(1),
	       &smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
	       AT91_SMC_PULSE_NRD(3) | AT91_SMC_PULSE_NCS_RD(5),
	       &smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(8) | AT91_SMC_CYCLE_NRD(8),
	       &smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
	       AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
	       AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
	       AT91_SMC_MODE_DBW_8 |
#endif
	       AT91_SMC_MODE_TDF_CYCLE(3),
	       &smc->cs[3].mode);

	writel(1 << ATMEL_ID_PIOE, &pmc->pcer);
}
#endif

#ifdef CONFIG_CMD_USB
static void at91sama5ek_usb_hw_init(void)
{
	at91_set_pio_output(AT91_PIO_PORTD, 26, 0);
	at91_set_pio_output(AT91_PIO_PORTD, 27, 0);
}
#endif

#ifdef CONFIG_MACB
static void at91sama5ek_macb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable clock */
	writel(1 << (ATMEL_ID_EMAC-32), &pmc->pcer1);

	/* And the pins. */
	at91_macb_hw_init();
}
#endif

#ifdef CONFIG_GMACB
static void at91sama5ek_gmacb_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/* Enable clock */
	writel(1 << (ATMEL_ID_GMAC-32), &pmc->pcer1);
	writel(1 << ATMEL_ID_PIOB, &pmc->pcer);

	/* And the pins. */
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

static void at91sama5ek_lcd_hw_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	at91_set_a_periph(AT91_PIO_PORTA, 24, 0);	/* LCDPWM */
	at91_set_a_periph(AT91_PIO_PORTA, 25, 0);	/* LCDDISP */
	at91_set_a_periph(AT91_PIO_PORTA, 26, 0);	/* LCDVSYNC */
	at91_set_a_periph(AT91_PIO_PORTA, 27, 0);	/* LCDHSYNC */
	at91_set_a_periph(AT91_PIO_PORTA, 28, 0);	/* LCDDOTCK */
	at91_set_a_periph(AT91_PIO_PORTA, 29, 0);	/* LCDDEN */

	at91_set_a_periph(AT91_PIO_PORTA,  0, 0);	/* LCDD0 */
	at91_set_a_periph(AT91_PIO_PORTA,  1, 0);	/* LCDD1 */
	at91_set_a_periph(AT91_PIO_PORTA,  2, 0);	/* LCDD2 */
	at91_set_a_periph(AT91_PIO_PORTA,  3, 0);	/* LCDD3 */
	at91_set_a_periph(AT91_PIO_PORTA,  4, 0);	/* LCDD4 */
	at91_set_a_periph(AT91_PIO_PORTA,  5, 0);	/* LCDD5 */
	at91_set_a_periph(AT91_PIO_PORTA,  6, 0);	/* LCDD6 */
	at91_set_a_periph(AT91_PIO_PORTA,  7, 0);	/* LCDD7 */
	at91_set_a_periph(AT91_PIO_PORTA,  8, 0);	/* LCDD8 */
	at91_set_a_periph(AT91_PIO_PORTA,  9, 0);	/* LCDD9 */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 0);	/* LCDD10 */
	at91_set_a_periph(AT91_PIO_PORTA, 11, 0);	/* LCDD11 */
	at91_set_a_periph(AT91_PIO_PORTA, 12, 0);	/* LCDD12 */
	at91_set_a_periph(AT91_PIO_PORTA, 13, 0);	/* LCDD13 */
	at91_set_a_periph(AT91_PIO_PORTA, 14, 0);	/* LCDD14 */
	at91_set_a_periph(AT91_PIO_PORTA, 15, 0);	/* LCDD15 */
	at91_set_c_periph(AT91_PIO_PORTC, 14, 0);	/* LCDD16 */
	at91_set_c_periph(AT91_PIO_PORTC, 13, 0);	/* LCDD17 */
	at91_set_c_periph(AT91_PIO_PORTC, 12, 0);	/* LCDD18 */
	at91_set_c_periph(AT91_PIO_PORTC, 11, 0);	/* LCDD19 */
	at91_set_c_periph(AT91_PIO_PORTC, 10, 0);	/* LCDD20 */
	at91_set_c_periph(AT91_PIO_PORTC, 15, 0);	/* LCDD21 */
	at91_set_c_periph(AT91_PIO_PORTE, 27, 0);	/* LCDD22 */
	at91_set_c_periph(AT91_PIO_PORTE, 28, 0);	/* LCDD23 */

	writel(1 << (ATMEL_ID_LCDC - 32), &pmc->pcer1);

	gd->fb_base = CONFIG_AT91SAMA5_LCD_BASE;
}

#ifdef CONFIG_LCD_INFO
#include <nand.h>
#include <version.h>

void lcd_show_board_info(void)
{
	ulong dram_size, nand_size;
	int i;
	char temp[32];

	lcd_printf ("%s\n", U_BOOT_VERSION);
	lcd_printf ("(C) 2008 ATMEL Corp\n");
	lcd_printf ("at91support@atmel.com\n");
	lcd_printf ("%s CPU at %s MHz\n",
		ATMEL_CPU_NAME,
		strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i].size;
	lcd_printf ("  %ld MB SDRAM, %ld MB NAND\n",
		dram_size >> 20,
		nand_size >> 20 );
}
#endif /* CONFIG_LCD_INFO */
#endif

int board_early_init_f(void)
{
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

	/* arch number of AT91SAMA5EK-Board */
#if defined CONFIG_AT91SAMA5EK
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAMA5EK;
#endif

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sama5ek_nand_hw_init();
#endif
#ifdef CONFIG_CMD_USB
	at91sama5ek_usb_hw_init();
#endif
#ifdef CONFIG_SYS_USE_DATAFLASH
	at91_spi0_hw_init(1 << 0);
#endif
#ifdef CONFIG_ATMEL_SPI
	at91_spi0_hw_init(1 << 4);
#endif
#ifdef CONFIG_MACB
	at91sama5ek_macb_hw_init();
#endif
#ifdef CONFIG_GMACB
	at91sama5ek_gmacb_hw_init();
#endif
#ifdef CONFIG_LCD
	at91sama5ek_lcd_hw_init();
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
	rc = macb_eth_initialize(0, (void *)ATMEL_BASE_EMAC, 0x00);
#endif
#ifdef CONFIG_GMACB
	rc = gmacb_eth_initialize(0, (void *)ATMEL_BASE_GMAC, 0x00);
#endif
	return rc;
}

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
