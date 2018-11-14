// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Atmel Corporation
 *		      Bo Shen <voice.shen@atmel.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/at91_rstc.h>
#include <asm/arch/at91_wdt.h>
#include <asm/arch/clk.h>
#include <spl.h>

#define PLLA_DIV 1
#define PLLA_COUNT 0x3f
#define PLLA_LOOP_FILTER 0
#define PLLA_CLOCK 200000000
#define PLLA_FRACR(_p, _q) ((unsigned int)((((unsigned long)(_p)) << 22) / (_q)))

#define VDDIOM_1V8_OUT_Z_CALN_TYP 4
#define VDDIOM_1V8_OUT_Z_CALP_TYP 10

static void switch_to_main_crystal_osc(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;
	u32 tmp;

	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_OSCOUNT(0xff);
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_MOSCEN;
	tmp |= AT91_PMC_MOR_OSCOUNT(8);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);
	while (!(readl(&pmc->sr) & AT91_PMC_IXR_MOSCS))
		;

#if defined(CONFIG_SAMA5D2)
	/* Enable a measurement of the external oscillator */
	tmp = readl(&pmc->mcfr);
	tmp |= AT91_PMC_MCFR_CCSS_XTAL_OSC;
	tmp |= AT91_PMC_MCFR_RCMEAS;
	writel(tmp, &pmc->mcfr);

	while (!(readl(&pmc->mcfr) & AT91_PMC_MCFR_MAINRDY))
		;

	if (!(readl(&pmc->mcfr) & AT91_PMC_MCFR_MAINF_MASK))
		hang();
#endif

	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_OSCBYPASS;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);

	tmp = readl(&pmc->mor);
	tmp |= AT91_PMC_MOR_MOSCSEL;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);

	while (!(readl(&pmc->sr) & AT91_PMC_IXR_MOSCSELS))
		;

#if !defined(CONFIG_SAMA5D2)
	/* Wait until MAINRDY field is set to make sure main clock is stable */
	while (!(readl(&pmc->mcfr) & AT91_PMC_MAINRDY))
		;
#endif

#if !defined(CONFIG_SAMA5D4) && !defined(CONFIG_SAMA5D2)
	tmp = readl(&pmc->mor);
	tmp &= ~AT91_PMC_MOR_MOSCRCEN;
	tmp &= ~AT91_PMC_MOR_KEY(0xff);
	tmp |= AT91_PMC_MOR_KEY(0x37);
	writel(tmp, &pmc->mor);
#endif
}

__weak void matrix_init(void)
{
	/* This only be used for sama5d4 soc now */
}

__weak void redirect_int_from_saic_to_aic(void)
{
	/* This only be used for sama5d4 soc now */
}

/* empty stub to satisfy current lowlevel_init, can be removed any time */
void s_init(void)
{
}

void board_init_f(ulong dummy)
{
	int ret;
#if defined(CONFIG_SAM9X60)
    struct _pmc_plla_cfg plla_config;
#endif

	switch_to_main_crystal_osc();

#if !defined(CONFIG_AT91SAM9_WATCHDOG)
	/* disable watchdog */
	at91_disable_wdt();
#endif

#if defined(CONFIG_SAM9X60)
    /* Configure & Enable PLLA */
    plla_config.mul = 49;
    plla_config.div = PLLA_DIV;
    plla_config.count = PLLA_COUNT;
    plla_config.fracr = 0;
    plla_config.loop_filter = PLLA_LOOP_FILTER;
    pmc_sam9x60_cfg_pll(PLL_ID_PLLA, &plla_config);

    pmc_set_mck_divider(AT91_PMC_MCKR_MDIV_3);
    pmc_set_mck_prescaler(AT91_PMC_MCKR_PRES_1);

    /* switch mck to plla */
    pmc_switch_mck_to_pll();

#else

#ifdef CONFIG_SAMA5D2
	configure_2nd_sram_as_l2_cache();
#endif

	/* PMC configuration */
	at91_pmc_init();
#endif

	at91_clock_init(CONFIG_SYS_AT91_MAIN_CLOCK);

	matrix_init();

	redirect_int_from_saic_to_aic();

	timer_init();

	board_early_init_f();

	mem_init();

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

}
