/*
 * (C) Copyright 2007-2008
 * Stelian Pop <stelian.pop@leadtechdesign.com>
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
#include <asm/sizes.h>
#include <asm/arch/at91sam9x5.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>
#include <asm/arch/hardware.h>
//#include <lcd.h>
//#include <atmel_lcdc.h>
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
static void at91sam9x5ek_nand_hw_init(void)
{
	unsigned long csa;

	/* Enable CS3 */
	csa = at91_sys_read(AT91_MATRIX_EBICSA);
	
#if CONFIG_SYS_NAND_NFD0_ON_D16	
	csa |= AT91_MATRIX_NFD0_ON_D16;
#ifndef CONFIG_SYS_NAND_DBW_16
	csa |= AT91_MATRIX_MP_ON;
#endif
#else
	csa &= ~AT91_MATRIX_NFD0_ON_D16;
	csa &= ~AT91_MATRIX_MP_ON;
#endif
	/* Configure IO drive */
	csa &= ~AT91_MATRIX_EBI_EBI_IOSR;

	at91_sys_write(AT91_MATRIX_EBICSA,
		       csa | AT91_MATRIX_EBI_CS3A_SMC_NANDFLASH);

	/* Configure SMC CS3 for NAND/SmartMedia */
	at91_sys_write(AT91_SMC_SETUP(3),
		       AT91_SMC_NWESETUP_(2) | AT91_SMC_NCS_WRSETUP_(0) |
		       AT91_SMC_NRDSETUP_(2) | AT91_SMC_NCS_RDSETUP_(0));
	at91_sys_write(AT91_SMC_PULSE(3),
		       AT91_SMC_NWEPULSE_(4) | AT91_SMC_NCS_WRPULSE_(4) |
		       AT91_SMC_NRDPULSE_(4) | AT91_SMC_NCS_RDPULSE_(4));
	at91_sys_write(AT91_SMC_CYCLE(3),
		       AT91_SMC_NWECYCLE_(7) | AT91_SMC_NRDCYCLE_(7));
	at91_sys_write(AT91_SMC_MODE(3),
		       AT91_SMC_READMODE | AT91_SMC_WRITEMODE |
		       AT91_SMC_EXNWMODE_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		       AT91_SMC_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		       AT91_SMC_DBW_8 |
#endif
		       AT91_SMC_TDF_(3));

	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9X5_ID_PIOCD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);

	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);
	
	at91_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* ALE */
	at91_set_a_periph(AT91_PIO_PORTD, 3, 1);	/* CLE */
	
#if CONFIG_SYS_NAND_NFD0_ON_D16
	at91_set_a_periph(AT91_PIO_PORTD, 6, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 7, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 8, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 9, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 10, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 11, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 12, 1);
	at91_set_a_periph(AT91_PIO_PORTD, 13, 1);
#endif
}
#endif

#ifdef CONFIG_RESET_PHY_R
void reset_phy(void)
{
#ifdef CONFIG_MACB
	/*
	 * Initialize ethernet HW addr prior to starting Linux,
	 * needed for nfsroot
	 */
	eth_init(gd->bd);
#endif
}
#endif

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_MACB
	rc = macb_eth_initialize(0, (void *)AT91SAM9X5_BASE_EMAC0, 0x00);
#endif
	return rc;
}

#ifdef CONFIG_MACB
static void at91sam9x5ek_macb_hw_init(void)
{
	/* Enable clock */
	at91_sys_write(AT91_PMC_PCER, 1 << AT91SAM9X5_ID_EMAC);
	at91_macb_hw_init();
}
#endif

int board_init(void)
{
	/* Enable Ctrlc */
	console_init_f();

	/* arch number of AT91SAM9X5EK-Board */
	//gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9X5EK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	at91_serial_hw_init();
#ifdef CONFIG_CMD_NAND
	at91sam9x5ek_nand_hw_init();
#endif

#ifdef CONFIG_MACB
	at91sam9x5ek_macb_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_SIZE;
	return 0;
}


