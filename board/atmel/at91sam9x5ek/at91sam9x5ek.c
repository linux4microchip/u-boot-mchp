/*
 * Copyright (C) 2012 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/at91sam9x5_matrix.h>
#include <asm/arch/at91sam9_smc.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/clk.h>
#include <asm/arch/gpio.h>
#include <debug_uart.h>
#include <atmel_lcd.h>
#include <nand.h>
#include <version.h>
#include <video.h>
#ifdef CONFIG_DM_VIDEO
#include <video_console.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */
/*
 * Miscelaneous platform dependent initialisations
 */
#ifdef CONFIG_CMD_NAND
static void at91sam9x5ek_nand_hw_init(void)
{
	struct at91_smc *smc = (struct at91_smc *)ATMEL_BASE_SMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	unsigned long csa;

	/* Enable CS3 */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS3A_SMC_SMARTMEDIA;
	/* NAND flash on D16 */
	csa |= AT91_MATRIX_NFD0_ON_D16;

	/* Configure IO drive */
	csa &= ~AT91_MATRIX_EBI_EBI_IOSR_NORMAL;

	writel(csa, &matrix->ebicsa);

	/* Configure SMC CS3 for NAND/SmartMedia */
	writel(AT91_SMC_SETUP_NWE(1) | AT91_SMC_SETUP_NCS_WR(0) |
		AT91_SMC_SETUP_NRD(1) | AT91_SMC_SETUP_NCS_RD(0),
		&smc->cs[3].setup);
	writel(AT91_SMC_PULSE_NWE(3) | AT91_SMC_PULSE_NCS_WR(5) |
		AT91_SMC_PULSE_NRD(4) | AT91_SMC_PULSE_NCS_RD(6),
		&smc->cs[3].pulse);
	writel(AT91_SMC_CYCLE_NWE(5) | AT91_SMC_CYCLE_NRD(6),
		&smc->cs[3].cycle);
	writel(AT91_SMC_MODE_RM_NRD | AT91_SMC_MODE_WM_NWE |
		AT91_SMC_MODE_EXNW_DISABLE |
#ifdef CONFIG_SYS_NAND_DBW_16
		AT91_SMC_MODE_DBW_16 |
#else /* CONFIG_SYS_NAND_DBW_8 */
		AT91_SMC_MODE_DBW_8 |
#endif
		AT91_SMC_MODE_TDF_CYCLE(1),
		&smc->cs[3].mode);

	at91_periph_clk_enable(ATMEL_ID_PIOCD);

	/* Configure RDY/BSY */
	at91_set_gpio_input(CONFIG_SYS_NAND_READY_PIN, 1);
	/* Enable NandFlash */
	at91_set_gpio_output(CONFIG_SYS_NAND_ENABLE_PIN, 1);

	at91_pio3_set_a_periph(AT91_PIO_PORTD, 0, 1);	/* NAND OE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 1, 1);	/* NAND WE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 2, 1);	/* NAND ALE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 3, 1);	/* NAND CLE */
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 6, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 7, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 8, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 9, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 10, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 11, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 12, 1);
	at91_pio3_set_a_periph(AT91_PIO_PORTD, 13, 1);
}
#endif

#if defined(CONFIG_DM_VIDEO) && defined(CONFIG_ATMEL_HLCD)
static int video_show_board_logo_info(void)
{
	ulong dram_size, nand_size;
	int i;
	u32 len = 0;
	char buf[255];
	char *corp = "2017 Microchip Technology Inc.\n";
	char *support = "at91support@atmel.com\n";
	char temp[32];
	struct udevice *dev, *con;
	const char *s;
	vidinfo_t logo_info;
	int ret;

	get_microchip_logo_info(&logo_info);

	len += sprintf(&buf[len], "%s\n", U_BOOT_VERSION);
	memcpy(&buf[len], corp, strlen(corp));
	len += strlen(corp);
	memcpy(&buf[len], support, strlen(support));
	len += strlen(support);
	len += sprintf(&buf[len], "%s CPU at %s MHz\n", get_cpu_name(),
			strmhz(temp, get_cpu_clk_rate()));

	dram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		dram_size += gd->bd->bi_dram[i].size;
	nand_size = 0;
	for (i = 0; i < CONFIG_SYS_MAX_NAND_DEVICE; i++)
		nand_size += nand_info[i]->size;

	len += sprintf(&buf[len], "%ld MB SDRAM, %ld MB NAND\n",
		       dram_size >> 20, nand_size >> 20);

	ret = uclass_get_device(UCLASS_VIDEO, 0, &dev);
	if (ret)
		return ret;

	ret = video_bmp_display(dev, logo_info.logo_addr,
				logo_info.logo_x_offset,
				logo_info.logo_y_offset, false);
	if (ret)
		return ret;

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con);
	if (ret)
		return ret;

	vidconsole_position_cursor(con, 0, logo_info.logo_height);
	for (s = buf, i = 0; i < len; s++, i++)
		vidconsole_put_char(con, *s);

	return 0;
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_DM_VIDEO
	video_show_board_logo_info();
#endif
	return 0;
}
#endif

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	at91_seriald_hw_init();
}
#endif

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
#ifdef CONFIG_DEBUG_UART
	debug_uart_init();
#endif
	return 0;
}
#endif

int board_init(void)
{
	/* arch number of AT91SAM9X5EK-Board */
	gd->bd->bi_arch_number = MACH_TYPE_AT91SAM9X5EK;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_CMD_NAND
	at91sam9x5ek_nand_hw_init();
#endif

#if defined(CONFIG_USB_OHCI_NEW) || defined(CONFIG_USB_EHCI)
	at91_uhp_hw_init();
#endif
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *) CONFIG_SYS_SDRAM_BASE,
					CONFIG_SYS_SDRAM_SIZE);
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>
#include <nand.h>

void at91_spl_board_init(void)
{
#ifdef CONFIG_SYS_USE_MMC
	at91_mci_hw_init();
#elif CONFIG_SYS_USE_NANDFLASH
	at91sam9x5ek_nand_hw_init();
#endif
}

#include <asm/arch/atmel_mpddrc.h>
static void ddr2_conf(struct atmel_mpddrc_config *ddr2)
{
	ddr2->md = (ATMEL_MPDDRC_MD_DBW_16_BITS | ATMEL_MPDDRC_MD_DDR2_SDRAM);

	ddr2->cr = (ATMEL_MPDDRC_CR_NC_COL_10 |
		    ATMEL_MPDDRC_CR_NR_ROW_13 |
		    ATMEL_MPDDRC_CR_CAS_DDR_CAS3 |
		    ATMEL_MPDDRC_CR_NB_8BANKS |
		    ATMEL_MPDDRC_CR_DECOD_INTERLEAVED);

	ddr2->rtr = 0x411;

	ddr2->tpr0 = (6 << ATMEL_MPDDRC_TPR0_TRAS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRCD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWR_OFFSET |
		      8 << ATMEL_MPDDRC_TPR0_TRC_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRP_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TRRD_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TWTR_OFFSET |
		      2 << ATMEL_MPDDRC_TPR0_TMRD_OFFSET);

	ddr2->tpr1 = (2 << ATMEL_MPDDRC_TPR1_TXP_OFFSET |
		      200 << ATMEL_MPDDRC_TPR1_TXSRD_OFFSET |
		      19 << ATMEL_MPDDRC_TPR1_TXSNR_OFFSET |
		      18 << ATMEL_MPDDRC_TPR1_TRFC_OFFSET);

	ddr2->tpr2 = (7 << ATMEL_MPDDRC_TPR2_TFAW_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET |
		      3 << ATMEL_MPDDRC_TPR2_TRPA_OFFSET |
		      7 << ATMEL_MPDDRC_TPR2_TXARDS_OFFSET |
		      2 << ATMEL_MPDDRC_TPR2_TXARD_OFFSET);
}

void mem_init(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	struct at91_matrix *matrix = (struct at91_matrix *)ATMEL_BASE_MATRIX;
	struct atmel_mpddrc_config ddr2;
	unsigned long csa;

	ddr2_conf(&ddr2);

	/* enable DDR2 clock */
	writel(AT91_PMC_DDR, &pmc->scer);

	/* Chip select 1 is for DDR2/SDRAM */
	csa = readl(&matrix->ebicsa);
	csa |= AT91_MATRIX_EBI_CS1A_SDRAMC;
	csa &= ~AT91_MATRIX_EBI_DBPU_OFF;
	csa |= AT91_MATRIX_EBI_DBPD_OFF;
	csa |= AT91_MATRIX_EBI_EBI_IOSR_NORMAL;
	writel(csa, &matrix->ebicsa);

	/* DDRAM2 Controller initialize */
	ddr2_init(ATMEL_BASE_DDRSDRC, ATMEL_BASE_CS1, &ddr2);
}
#endif
