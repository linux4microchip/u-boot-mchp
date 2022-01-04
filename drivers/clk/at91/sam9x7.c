// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries
 *
 * Author: Varshini Rajendran <varshini.rajendran@microchip.com>
 *
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <dt-bindings/clk/at91.h>
#include <linux/clk-provider.h>

#include "pmc.h"

/**
 * Clock identifiers to be used in conjunction with macros like
 * AT91_TO_CLK_ID()
 *
 * @ID_MD_SLCK:         	TD slow clock identifier
 * @ID_TD_SLCK:         	MD slow clock identifier
 * @ID_MAIN_XTAL:       	Main Xtal clock identifier
 * @ID_MAIN_RC:         	Main RC clock identifier
 * @ID_MAIN_RC_OSC:     	Main RC Oscillator clock identifier
 * @ID_MAIN_OSC:        	Main Oscillator clock identifier
 * @ID_MAINCK:          	MAINCK clock identifier
 * @ID_PLL_U_FRAC:      	UPLL fractional clock identifier
 * @ID_PLL_U_DIV:       	UPLL divider clock identifier
 * @ID_PLL_A_FRAC:      	APLL fractional clock identifier
 * @ID_PLL_A_DIV:       	APLL divider clock identifier
 * @ID_PLL_A_2_FRAC:    	PLLA DIV2 fractional clock identifier
 * @ID_PLL_A_2_DIV:     	PLLA DIV2 divider clock identifier
 * @ID_PLL_AUDIO_FRAC:  	Audio PLL fractional clock identifier
 * @ID_PLL_AUDIO_DIVPMC:   	Audio PLL divider clock identifier
 * @ID_PLL_AUDIO_DIVIO:	Audio PLL IO divider clock identifier
 * @ID_PLL_LVDS_FRAC:   	LVDS PLL fractional clock identifier
 * @ID_PLL_LVDS_DIV:    	LVDS PLL divider clock identifier

 * @ID_MCK_DIV: 		MCK DIV clock identifier

 * @ID_UTMI:			UTMI clock identifier

 * @ID_PROG0:			Programmable 0 clock identifier
 * @ID_PROG1:			Programmable 1 clock identifier
 * @ID_PROG2:			Programmable 2 clock identifier
 * @ID_PROG3:			Programmable 3 clock identifier
 * @ID_PROG4:			Programmable 4 clock identifier
 * @ID_PROG5:			Programmable 5 clock identifier
 * @ID_PROG6:			Programmable 6 clock identifier

 * @ID_PCK0:			PCK0 system clock identifier
 * @ID_PCK1:			PCK1 system clock identifier
 * @ID_PCK2:			PCK2 system clock identifier
 * @ID_PCK3:			PCK3 system clock identifier
 * @ID_PCK4:			PCK4 system clock identifier
 * @ID_PCK5:			PCK5 system clock identifier
 * @ID_PCK6:			PCK6 system clock identifier
 * @ID_DDR:			DDR system clock identifier
 * @ID_QSPI:			QSPI system clock identifier
 *
 * @ID_MCK_PRES:		MCK PRES clock identifier
 *
 * Note: if changing the values of this enums please sync them with
 *       device tree
 */
enum pmc_clk_ids {
        ID_MD_SLCK              = 0,
        ID_TD_SLCK              = 1,
        ID_MAIN_XTAL            = 2,
        ID_MAIN_RC              = 3,
        ID_MAIN_RC_OSC          = 4,
        ID_MAIN_OSC             = 5,
        ID_MAINCK               = 6,

        ID_PLL_U_FRAC           = 7,
        ID_PLL_U_DIV            = 8,
        ID_PLL_A_FRAC           = 9,
        ID_PLL_A_DIV            = 10,
        ID_PLL_A_2_FRAC         = 11,
        ID_PLL_A_2_DIV          = 12,
        ID_PLL_AUDIO_FRAC       = 13,
        ID_PLL_AUDIO_DIVPMC     = 14,
        ID_PLL_AUDIO_DIVIO      = 15,
        ID_PLL_LVDS_FRAC        = 16,
        ID_PLL_LVDS_DIV         = 17,

        ID_MCK_DIV              = 18,

        ID_UTMI                 = 19,

        ID_PROG0                = 20,
        ID_PROG1                = 21,
        ID_PROG2                = 22,
        ID_PROG3                = 23,
        ID_PROG4                = 24,
        ID_PROG5                = 25,
        ID_PROG6                = 26,

        ID_PCK0                 = 27,
        ID_PCK1                 = 28,
        ID_PCK2                 = 29,
        ID_PCK3                 = 30,
        ID_PCK4                 = 31,
        ID_PCK5                 = 32,
        ID_PCK6                 = 33,

        ID_DDR                  = 34,
        ID_QSPI                 = 35,

        ID_MCK_PRES             = 36,

        ID_MAX,
};

/**
 * PLL type identifiers
 * @PLL_TYPE_FRAC:	fractional PLL identifier
 * @PLL_TYPE_DIV:	divider PLL identifier
 */
enum pll_type {
	PLL_TYPE_FRAC,
	PLL_TYPE_DIV,
};

/* Clock names used as parents for multiple clocks. */
static const char *clk_names[] = {
	[ID_MAIN_RC_OSC]	= "main_rc_osc",
	[ID_MAIN_OSC]		= "main_osc",
	[ID_MAINCK]		= "mainck",
	[ID_PLL_U_DIV]		= "upll_divpmcck",
	[ID_PLL_A_DIV]		= "plla_divpmcck",
	[ID_PLL_A_2_DIV]	= "plla_div2pmcck",
	[ID_PLL_AUDIO_DIVPMC]	= "pll_audio_divpmcck",
	[ID_PLL_AUDIO_DIVIO]	= "pll_audio_diviock",
	[ID_PLL_LVDS_DIV]	= "pll_lvds_divpmcck",
	[ID_MCK_PRES]		= "mck_pres",
	[ID_MCK_DIV]		= "mck_div",
};

/* Fractional PLL output range. */
static const struct clk_range plla_outputs[] = {
	{ .min = 2343750, .max = 1600000000 },
};

static const struct clk_range upll_outputs[] = {
	{ .min = 300000000, .max = 500000000 },
};

/* PLL characteristics. */
static const struct clk_pll_characteristics apll_characteristics = {
	.input = { .min = 20000000, .max = 50000000 },
	.num_output = ARRAY_SIZE(plla_outputs),
	.output = plla_outputs,
};

static const struct clk_pll_characteristics upll_characteristics = {
	.input = { .min = 20000000, .max = 50000000 },
	.num_output = ARRAY_SIZE(upll_outputs),
	.output = upll_outputs,
	.upll = true,
};

/* Layout for fractional PLLs. */
static const struct clk_pll_layout pll_layout_frac = {
	.mul_mask = GENMASK(31, 24),
	.frac_mask = GENMASK(21, 0),
	.mul_shift = 24,
	.frac_shift = 0,
};

/* Layout for DIV PLLs. */
static const struct clk_pll_layout pll_layout_divpmc = {
	.div_mask = GENMASK(7, 0),
	.endiv_mask = BIT(29),
	.div_shift = 0,
	.endiv_shift = 29,
};

/* Layout for DIVIO dividers. */
static const struct clk_pll_layout pll_layout_divio = {
        .div_mask       = GENMASK(19, 12),
        .endiv_mask     = BIT(30),
        .div_shift      = 12,
        .endiv_shift    = 30,
};

/* MCK characteristics. */
static const struct clk_master_characteristics mck_characteristics = {
	.output = { .min = 140000000, .max = 266666666 },
	.divisors = { 1, 2, 4, 3, 5},
	.have_div3_pres = 1,
};

/* MCK layout. */
static const struct clk_master_layout mck_layout = {
	.mask = 0x373,
	.pres_shift = 4,
	.offset = 0x28,
};

/* Programmable clock layout. */
static const struct clk_programmable_layout programmable_layout = {
	.pres_mask = 0xff,
	.pres_shift = 8,
	.css_mask = 0x1f,
	.have_slck_mck = 0,
	.is_pres_direct = 1,
};

/* Peripheral clock layout. */
static const struct clk_pcr_layout sam9x7_pcr_layout = {
	.offset = 0x88,
	.cmd = BIT(31),
	.gckcss_mask = GENMASK(12, 8),
	.pid_mask = GENMASK(6, 0),
};

/**
 * PLL clocks description
 * @n:		clock name
 * @p:		clock parent
 * @l:		clock layout
 * @t:		clock type
 * @f:		true if clock is fixed and not changeable by driver
 * @id:		clock id corresponding to PLL driver
 * @cid:	clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	const struct clk_pll_layout *l;
	const struct clk_pll_characteristics *c;
	u8 t;
	u8 f;
	u8 id;
	u8 cid;
} sam9x7_plls[] = {
	{
		.n = "plla_fracck",
		.p = "mainck",
		.l = &pll_layout_frac,
		.c = &apll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 0,
		.cid = ID_PLL_A_FRAC,
	},

	{
		.n = "plla_divpmcck",
		.p = "plla_fracck",
		.l = &pll_layout_divpmc,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 0,
		.cid = ID_PLL_A_DIV,
	},

	{
		.n = "upll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.c = &upll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 1,
		.cid = ID_PLL_U_FRAC,
	},

	{
		.n = "upll_divpmcck",
		.p = "upll_fracck",
		.l = &pll_layout_divpmc,
		.c = &upll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 1,
		.cid = ID_PLL_U_DIV,
	},

	{
		.n = "audiopll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.c = &apll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 2,
		.cid = ID_PLL_AUDIO_FRAC,
	},

	{
		.n = "audiopll_divpmcck",
		.p = "audiopll_fracck",
		.l = &pll_layout_divpmc,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 2,
		.cid = ID_PLL_AUDIO_DIVPMC,
	},

	{
		.n = "audiopll_diviock",
		.p = "audiopll_fracck",
		.l = &pll_layout_divio,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 2,
		.cid = ID_PLL_AUDIO_DIVIO,
	},

	{
		.n = "lvdspll_fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.c = &apll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 3,
		.cid = ID_PLL_LVDS_FRAC,
	},

	{
		.n = "lvdspll_divpmcck",
		.p = "lvdspll_fracck",
		.l = &pll_layout_divpmc,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 3,
		.cid = ID_PLL_LVDS_DIV,
	},

	{
		.n = "plla_div2fracck",
		.p = "main_osc",
		.l = &pll_layout_frac,
		.c = &apll_characteristics,
		.t = PLL_TYPE_FRAC,
		.f = 1,
		.id = 4,
		.cid = ID_PLL_A_2_FRAC,
	},

	{
		.n = "plla_div2pmcck",
		.p = "plla_fracck",
		.l = &pll_layout_divpmc,
		.c = &apll_characteristics,
		.t = PLL_TYPE_DIV,
		.f = 1,
		.id = 4,
		.cid = ID_PLL_A_2_DIV,
	},

};

/**
 * Programmable clock description
 * @n:			clock name
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	u8 cid;
} sam9x7_prog[] = {
	{ .n = "prog0", .cid = ID_PROG0, },
	{ .n = "prog1", .cid = ID_PROG1, },
	{ .n = "prog2", .cid = ID_PROG2, },
	{ .n = "prog3", .cid = ID_PROG3, },
	{ .n = "prog4", .cid = ID_PROG4, },
	{ .n = "prog5", .cid = ID_PROG5, },
	{ .n = "prog6", .cid = ID_PROG6, },
};

/* Mux table for programmable clocks. */
static u32 sam9x7_prog_mux_table[] = { 0, 1, 2, 3, 4, 5, 6, };

/**
 * System clock description
 * @n:			clock name
 * @p:			parent clock name
 * @id:			clock id corresponding to system clock driver
 * @cid:		clock id corresponding to clock subsystem
 */
static const struct {
	const char *n;
	const char *p;
	u8 id;
	u8 cid;
} sam9x7_systemck[] = {
	{ .n = "ddrck", .p = "mck_pres", .id = 2, .cid = ID_DDR, },
	{ .n = "pck0",  .p = "prog0",    .id = 8, .cid = ID_PCK0, },
	{ .n = "pck1",  .p = "prog1",    .id = 9, .cid = ID_PCK1, },
	{ .n = "pck2",  .p = "prog2",    .id = 10, .cid = ID_PCK2, },
	{ .n = "pck3",  .p = "prog3",    .id = 11, .cid = ID_PCK3, },
	{ .n = "pck4",  .p = "prog4",    .id = 12, .cid = ID_PCK4, },
	{ .n = "pck5",  .p = "prog5",    .id = 13, .cid = ID_PCK5, },
	{ .n = "pck6",  .p = "prog6",    .id = 14, .cid = ID_PCK6, },
};

/**
 * Peripheral clock description
 * @n:		clock name
 * @id:		clock id
 */
static const struct {
	const char *n;
	u8 id;
} sam9x7_periphck[] = {
	{ .n = "pioA_clk",     .id = 2, },
	{ .n = "pioB_clk",     .id = 3, },
	{ .n = "pioC_clk",     .id = 4, },
	{ .n = "flex0_clk",    .id = 5, },
	{ .n = "flex1_clk",    .id = 6, },
	{ .n = "flex2_clk",    .id = 7, },
	{ .n = "flex3_clk",    .id = 8, },
	{ .n = "flex6_clk",    .id = 9, },
	{ .n = "flex7_clk",    .id = 10, },
	{ .n = "flex8_clk",    .id = 11, },
	{ .n = "sdmmc0_clk",   .id = 12, },
	{ .n = "flex4_clk",    .id = 13, },
	{ .n = "flex5_clk",    .id = 14, },
	{ .n = "flex9_clk",    .id = 15, },
	{ .n = "flex10_clk",   .id = 16, },
	{ .n = "tcb0_clk",     .id = 17, },
	{ .n = "pwm_clk",      .id = 18, },
	{ .n = "adc_clk",      .id = 19, },
	{ .n = "dma0_clk",     .id = 20, },
	{ .n = "matrix_clk",   .id = 21, },
	{ .n = "uhphs_clk",    .id = 22, },
	{ .n = "udphs_clk",    .id = 23, },
	{ .n = "gmac_clk",     .id = 24, },
	{ .n = "lcd_clk",      .id = 25, },
	{ .n = "sdmmc1_clk",   .id = 26, },
	{ .n = "ssc_clk",      .id = 28, },
	{ .n = "mcan0_clk",    .id = 29, },
	{ .n = "mcan1_clk",    .id = 30, },
	{ .n = "flex11_clk",   .id = 32, },
	{ .n = "flex12_clk",   .id = 33, },
	{ .n = "i2s_clk",      .id = 34, },
	{ .n = "qspi_clk",     .id = 35, },
	{ .n = "gfx2d_clk",    .id = 36, },
	{ .n = "pit64b0_clk",  .id = 37, },
	{ .n = "trng_clk",     .id = 38, },
	{ .n = "aes_clk",      .id = 39, },
	{ .n = "tdes_clk",     .id = 40, },
	{ .n = "sha_clk",      .id = 41, },
	{ .n = "classd_clk",   .id = 42, },
	{ .n = "isi_clk",      .id = 43, },
	{ .n = "pioD_clk",     .id = 44, },
	{ .n = "tcb1_clk",     .id = 45, },
	{ .n = "dbgu_clk",     .id = 47, },
	{ .n = "mpddr_clk",    .id = 49, },
	{ .n = "csi2dc_clk",   .id = 52, },
	{ .n = "csi4l_clk",    .id = 53, },
	{ .n = "dsi4l_clk",    .id = 54, },
	{ .n = "mipiphy_clk",  .id = 55, },
	{ .n = "lvdsc_clk",    .id = 56, },
	{ .n = "pit64b1_clk",  .id = 58, },
	{ .n = "puf_clk",      .id = 59, },
	{ .n = "gmactsu_clk",  .id = 67, },
};

/**
 * Generic clock description
 * @n:			clock name
 * @ep:			extra parents parents names
 * @ep_mux_table:	extra parents mux table
 * @ep_clk_mux_table:	extra parents clock mux table (for CCF)
 * @r:			clock output range
 * @ep_count:		extra parents count
 * @id:			clock id
 */
static const struct {
	const char *n;
	const char *ep[8];
	const char ep_mux_table[8];
	const char ep_clk_mux_table[8];
	struct clk_range r;
	u8 ep_count;
	u8 id;
} sam9x7_gck[] = {
	{
        .n = "flex0_gclk",
        .id = 5,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex1_gclk",
        .id = 6,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex2_gclk",
        .id = 7,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex3_gclk",
        .id = 8,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex6_gclk",
        .id = 9,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex7_gclk",
        .id = 10,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex8_gclk",
        .id = 11,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "sdmmc0_gclk",
        .id = 12,
        .r = { .max = 105000000 },
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "flex4_gclk",
        .id = 13,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex5_gclk",
        .id = 14,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex9_gclk",
        .id = 15,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex10_gclk",
        .id = 16,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "tcb0_gclk",
        .id = 17,
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "adc_gclk",
        .id = 19,
        .ep = { "upll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 5, 8, },
        .ep_clk_mux_table = { ID_PLL_U_DIV, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "gmac_gclk",
        .id = 24,
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "lcd_gclk",
        .id = 25,
        .r = { .max = 75000000 },
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "sdmmc1_gclk",
        .id = 26,
        .r = { .max = 105000000 },
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "mcan0_gclk",
        .id = 29,
        .r = { .max = 80000000 },
        .ep = { "upll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 5, 8, },
        .ep_clk_mux_table = { ID_PLL_U_DIV, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "mcan1_gclk",
        .id = 30,
        .r = { .max = 80000000 },
        .ep = { "upll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 5, 8, },
        .ep_clk_mux_table = { ID_PLL_U_DIV, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "flex11_gclk",
        .id = 32,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "flex12_gclk",
        .id = 33,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "i2s_gclk",
        .id = 34,
        .r = { .max = 100000000 },
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "pit64b0_gclk",
        .id = 37,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "classd_gclk",
        .id = 42,
        .r = { .max = 100000000 },
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "tcb1_gclk",
        .id = 45,
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

	{
        .n = "dbgu_gclk",
        .id = 47,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "mipiphy_gclk",
        .id = 55,
        .r = { .max = 27000000 },
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "pit64b1_gclk",
        .id = 58,
        .ep = { "plla_div2pmcck", },
        .ep_mux_table = { 8, },
        .ep_clk_mux_table = { ID_PLL_A_2_DIV, },
        .ep_count = 1,
    },

	{
        .n = "gmac_tsu_gclk",
        .id = 67,
        .ep = { "audiopll_divpmcck", "plla_div2pmcck", },
        .ep_mux_table = { 6, 8, },
        .ep_clk_mux_table = { ID_PLL_AUDIO_DIVPMC, ID_PLL_A_2_DIV, },
        .ep_count = 2,
    },

};

#define prepare_mux_table(_allocs, _index, _dst, _src, _num, _label)	\
	do {								\
		int _i;							\
		(_dst) = kzalloc(sizeof(*(_dst)) * (_num), GFP_KERNEL);	\
		if (!(_dst)) {						\
			ret = -ENOMEM;					\
			goto _label;					\
		}							\
		(_allocs)[(_index)++] = (_dst);				\
		for (_i = 0; _i < (_num); _i++)				\
			(_dst)[_i] = (_src)[_i];			\
	} while (0)

static int sam9x7_clk_probe(struct udevice *dev)
{
	void __iomem *base = (void *)devfdt_get_addr_ptr(dev);
	unsigned int *clkmuxallocs[64], *muxallocs[64];
	const char *p[10];
	unsigned int cm[10], m[10], *tmpclkmux, *tmpmux;
	struct clk clk, *c;
	int ret, muxallocindex = 0, clkmuxallocindex = 0, i, j;
	static const struct clk_range r = { 0, 0 };

	if (!base)
		return -EINVAL;

	memset(muxallocs,    0, ARRAY_SIZE(muxallocs));
	memset(clkmuxallocs, 0, ARRAY_SIZE(clkmuxallocs));

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;

	clk_names[ID_TD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_TD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 1, &clk);
	if (ret)
		return ret;

	ret = clk_get_by_id(clk.id, &c);
	if (ret)
		return ret;

	clk_names[ID_MD_SLCK] = kmemdup(clk_hw_get_name(c),
					strlen(clk_hw_get_name(c)) + 1,
					GFP_KERNEL);
	if (!clk_names[ID_MD_SLCK])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 2, &clk);
	if (ret)
		return ret;

	clk_names[ID_MAIN_XTAL] = kmemdup(clk_hw_get_name(&clk),
					  strlen(clk_hw_get_name(&clk)) + 1,
					  GFP_KERNEL);
	if (!clk_names[ID_MAIN_XTAL])
		return -ENOMEM;

	ret = clk_get_by_index(dev, 3, &clk);
	if (ret)
		goto fail;

	clk_names[ID_MAIN_RC] = kmemdup(clk_hw_get_name(&clk),
					strlen(clk_hw_get_name(&clk)) + 1,
					GFP_KERNEL);
	if (ret)
		goto fail;

	/* Register main rc oscillator. */
	c = at91_clk_main_rc(base, clk_names[ID_MAIN_RC_OSC],
			     clk_names[ID_MAIN_RC]);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC), c);

	/* Register main oscillator. */
	c = at91_clk_main_osc(base, clk_names[ID_MAIN_OSC],
			      clk_names[ID_MAIN_XTAL], false);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC), c);

	/* Register mainck. */
	p[0] = clk_names[ID_MAIN_RC_OSC];
	p[1] = clk_names[ID_MAIN_OSC];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_RC_OSC);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAIN_OSC);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 2,
			  fail);
	c = at91_clk_sam9x5_main(base, clk_names[ID_MAINCK], p,
				 2, tmpclkmux, PMC_TYPE_CORE);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK), c);

	/* Register PLL fracs clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x7_plls); i++) {
		if (sam9x7_plls[i].t != PLL_TYPE_FRAC)
			continue;

		c = sam9x60_clk_register_frac_pll(base, sam9x7_plls[i].n,
						  sam9x7_plls[i].p,
						  sam9x7_plls[i].id,
						  sam9x7_plls[i].c,
						  sam9x7_plls[i].l,
						  sam9x7_plls[i].f);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x7_plls[i].cid), c);
	}

	/* Register PLL div clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x7_plls); i++) {
		if (sam9x7_plls[i].t != PLL_TYPE_DIV)
			continue;

		c = sam9x60_clk_register_div_pll(base, sam9x7_plls[i].n,
						 sam9x7_plls[i].p,
						 sam9x7_plls[i].id,
						 sam9x7_plls[i].c,
						 sam9x7_plls[i].l,
						 sam9x7_plls[i].f);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x7_plls[i].cid), c);
	}

	/* Register MCK pres clock. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_MAINCK];
	p[2] = clk_names[ID_PLL_A_DIV];
	p[3] = clk_names[ID_PLL_U_DIV];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm, 4,
			  fail);
	c = at91_clk_register_master_pres(base, clk_names[ID_MCK_PRES], p, 4,
					  &mck_layout, &mck_characteristics,
					  tmpclkmux);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_PRES), c);

	/* Register MCK div clock. */
	c = at91_clk_register_master_div(base, clk_names[ID_MCK_DIV],
					 clk_names[ID_MCK_PRES],
					 &mck_layout, &mck_characteristics);
	if (IS_ERR(c)) {
		ret = PTR_ERR(c);
		goto fail;
	}
	clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV), c);

	/* Register programmable clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK_DIV];
	p[4] = clk_names[ID_PLL_A_DIV];
	p[5] = clk_names[ID_PLL_U_DIV];
	p[6] = clk_names[ID_PLL_AUDIO_DIVPMC];
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV);
	cm[4] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_A_DIV);
	cm[5] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_U_DIV);
	cm[6] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_PLL_AUDIO_DIVPMC);
	for (i = 0; i < ARRAY_SIZE(sam9x7_prog); i++) {
		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  7, fail);

		c = at91_clk_register_programmable(base, sam9x7_prog[i].n, p,
						   7, i, &programmable_layout,
						   tmpclkmux,
						   sam9x7_prog_mux_table);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_CORE, sam9x7_prog[i].cid), c);
	}

	/* System clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x7_systemck); i++) {
		c = at91_clk_register_system(base, sam9x7_systemck[i].n,
					     sam9x7_systemck[i].p,
					     sam9x7_systemck[i].id);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_SYSTEM, sam9x7_systemck[i].cid),
		       c);
	}

	/* Peripheral clocks. */
	for (i = 0; i < ARRAY_SIZE(sam9x7_periphck); i++) {
		c = at91_clk_register_sam9x5_peripheral(base, &sam9x7_pcr_layout,
							sam9x7_periphck[i].n,
							clk_names[ID_MCK_DIV],
							sam9x7_periphck[i].id,
							&r);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_PERIPHERAL,
				      sam9x7_periphck[i].id), c);
	}

	/* Generic clocks. */
	p[0] = clk_names[ID_MD_SLCK];
	p[1] = clk_names[ID_TD_SLCK];
	p[2] = clk_names[ID_MAINCK];
	p[3] = clk_names[ID_MCK_DIV];
	m[0] = 0;
	m[1] = 1;
	m[2] = 2;
	m[3] = 3;
	cm[0] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MD_SLCK);
	cm[1] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_TD_SLCK);
	cm[2] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MAINCK);
	cm[3] = AT91_TO_CLK_ID(PMC_TYPE_CORE, ID_MCK_DIV);
	for (i = 0; i < ARRAY_SIZE(sam9x7_gck); i++) {
		for (j = 0; j < sam9x7_gck[i].ep_count; j++) {
			p[4 + j] = sam9x7_gck[i].ep[j];
			m[4 + j] = sam9x7_gck[i].ep_mux_table[j];
			cm[4 + j] = AT91_TO_CLK_ID(PMC_TYPE_CORE,
					sam9x7_gck[i].ep_clk_mux_table[j]);
		}
		prepare_mux_table(clkmuxallocs, clkmuxallocindex, tmpclkmux, cm,
				  4 + sam9x7_gck[i].ep_count, fail);
		prepare_mux_table(muxallocs, muxallocindex, tmpmux, m,
				  4 + sam9x7_gck[i].ep_count, fail);

		c = at91_clk_register_generic(base, &sam9x7_pcr_layout,
					      sam9x7_gck[i].n, p, tmpclkmux,
					      tmpmux, 4 + sam9x7_gck[i].ep_count, sam9x7_gck[i].id,
					      &sam9x7_gck[i].r);
		if (IS_ERR(c)) {
			ret = PTR_ERR(c);
			goto fail;
		}
		clk_dm(AT91_TO_CLK_ID(PMC_TYPE_GCK, sam9x7_gck[i].id), c);
	}

	return 0;

fail:
	for (i = 0; i < ARRAY_SIZE(muxallocs); i++)
		kfree(muxallocs[i]);

	for (i = 0; i < ARRAY_SIZE(clkmuxallocs); i++)
		kfree(clkmuxallocs[i]);

	return ret;
}

static const struct udevice_id sam9x7_clk_ids[] = {
	{ .compatible = "microchip,sam9x7-pmc" },
	{ /* Sentinel. */ },
};

U_BOOT_DRIVER(at91_sam9x7_pmc) = {
	.name = "at91-sam9x7-pmc",
	.id = UCLASS_CLK,
	.of_match = sam9x7_clk_ids,
	.ops = &at91_clk_ops,
	.probe = sam9x7_clk_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
