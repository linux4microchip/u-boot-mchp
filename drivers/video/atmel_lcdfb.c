/*
 * Driver for AT91/AT32 LCD Controller
 *
 * Copyright (C) 2007 Atmel Corporation
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <lcd.h>
#include <atmel_lcdc.h>
#if defined(CONFIG_AT91SAM9X5)
#include <atmel_9x5_lcdc.h>
#endif

int lcd_line_length;
int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;				/* Start of framebuffer memory	*/
void *lcd_console_address;		/* Start of console buffer	*/

short console_col;
short console_row;

/* configurable parameters */
#define ATMEL_LCDC_CVAL_DEFAULT		0xc8
#define ATMEL_LCDC_DMA_BURST_LEN	8
#ifndef ATMEL_LCDC_GUARD_TIME
#define ATMEL_LCDC_GUARD_TIME		1
#endif

#if defined(CONFIG_AT91SAM9263) || defined(CONFIG_AT91CAP9)
#define ATMEL_LCDC_FIFO_SIZE		2048
#else
#define ATMEL_LCDC_FIFO_SIZE		512
#endif

#define lcdc_readl(mmio, reg)		__raw_readl((mmio)+(reg))
#define lcdc_writel(mmio, reg, val)	__raw_writel((val), (mmio)+(reg))

void lcd_setcolreg(ushort regno, ushort red, ushort green, ushort blue)
{
#if defined(CONFIG_ATMEL_LCD_BGR555)
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (red >> 3) | ((green & 0xf8) << 2) | ((blue & 0xf8) << 7));
#else
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LUT(regno),
		    (blue >> 3) | ((green & 0xfc) << 3) | ((red & 0xf8) << 8));
#endif
}

#if defined(CONFIG_AT91SAM9X5)
void lcd_9x5_ctrl_init(void *lcdbase)
{
	unsigned long value;
	lcd_dma_desc *desc;

	/* Disable DISP signal */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDDIS, LCDC_LCDDIS_DISPDIS);
	while ((lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_DISPSTS))
		udelay(1);
	/* Disable synchronization */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDDIS, LCDC_LCDDIS_SYNCDIS);
	while ((lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_LCDSTS))
		udelay(1);
	/* Disable pixel clock */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDDIS, LCDC_LCDDIS_CLKDIS);
	while ((lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_CLKSTS))
		udelay(1);
	/* Disable PWM */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDDIS, LCDC_LCDDIS_PWMDIS);
	while ((lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_PWMSTS))
		udelay(1);

	/* Set pixel clock */
	value = get_lcdc_clk_rate(0) / panel_info.vl_clk;
	if (get_lcdc_clk_rate(0) % panel_info.vl_clk)
		value++;

	if (value < 1) {
		/* Using system clock as pixel clock */
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG0,
					LCDC_LCDCFG0_CLKDIV(0) | 0x1b5);
	} else {
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG0,
					LCDC_LCDCFG0_CLKDIV(value - 2) | 0x1b1);
	}

	/* Initialize control register 5 */
	value = 0;

	value |= LCDC_LCDCFG5_HSPOL;
	value |= LCDC_LCDCFG5_VSPOL;
	value |= LCDC_LCDCFG5_MODE_OUTPUT_24BPP;

	value |= LCDC_LCDCFG5_GUARDTIME(ATMEL_LCDC_GUARD_TIME);
	value |= (LCDC_LCDCFG5_DISPDLY | LCDC_LCDCFG5_VSPDLYS);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG5, value);

	/* Vertical & Horizontal Timing */
	value = LCDC_LCDCFG1_VSPW(panel_info.vl_vsync_len - 1);
	value |= LCDC_LCDCFG1_HSPW(panel_info.vl_hsync_len - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG1, value);

	value = LCDC_LCDCFG2_VBPW(panel_info.vl_lower_margin);
	value |= LCDC_LCDCFG2_VFPW(panel_info.vl_upper_margin - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG2, value);

	value = LCDC_LCDCFG3_HBPW(panel_info.vl_right_margin - 1);
	value |= LCDC_LCDCFG3_HFPW(panel_info.vl_left_margin - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG3, value);

	/* Display size */
	value = LCDC_LCDCFG4_RPF(panel_info.vl_row - 1);
	value |= LCDC_LCDCFG4_PPL(panel_info.vl_col - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCFG4, value);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG0, LCDC_BASECFG0_BLEN_AHB_INCR4 | LCDC_BASECFG0_DLBO);

	switch (NBITS(panel_info.vl_bpix)) {
	case 24:
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG1, LCDC_BASECFG1_RGBMODE_24BPP_RGB_888_PACKED);
		break;
	case 16:
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG1, LCDC_BASECFG1_RGBMODE_16BPP_RGB_565);
		break;
	default:
		BUG();
		break;
	}

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG2, LCDC_BASECFG2_XSTRIDE(0));
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG3, 0x555555);  /* Default color */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECFG4, LCDC_BASECFG4_DMA);

	/* Disable all interrupts */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDIDR, ~0UL);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASEIDR, ~0UL);

	/* Setup the DMA descriptor, this descriptor will loop to itself */
	desc = (lcd_dma_desc *)(lcdbase - 16);

	desc->address = lcdbase;
	/* Disable DMA transfer interrupt & descriptor loaded interrupt. */
	desc->control = LCDC_BASECTRL_ADDIEN | LCDC_BASECTRL_DSCRIEN
			| LCDC_BASECTRL_DMAIEN | LCDC_BASECTRL_DFETCH;
	desc->next = desc;

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASEADDR, desc->address);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECTRL, desc->control);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASENEXT, desc->next);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_BASECHER, LCDC_BASECHER_CHEN | LCDC_BASECHER_UPDATEEN);

	/* Enable LCD */
	value = lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDEN, value | LCDC_LCDEN_CLKEN);
	while (!(lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_CLKSTS))
		udelay(1);
	value = lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDEN, value | LCDC_LCDEN_SYNCEN);
	while (!(lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_LCDSTS))
		udelay(1);
	value = lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDEN, value | LCDC_LCDEN_DISPEN);
	while (!(lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_DISPSTS))
		udelay(1);
	value = lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDEN, value | LCDC_LCDEN_PWMEN);
	while (!(lcdc_readl(panel_info.mmio, ATMEL_LCDC_LCDSR) & LCDC_LCDSR_PWMSTS))
		udelay(1);
}
#endif

void lcd_ctrl_init(void *lcdbase)
{
	unsigned long value;

#if defined(CONFIG_AT91SAM9X5)
	return lcd_9x5_ctrl_init(lcdbase);
#endif

	/* Turn off the LCD controller and the DMA controller */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_PWRCON,
		    ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET);

	/* Wait for the LCDC core to become idle */
	while (lcdc_readl(panel_info.mmio, ATMEL_LCDC_PWRCON) & ATMEL_LCDC_BUSY)
		udelay(10);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, 0);

	/* Reset LCDC DMA */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMARST);

	/* ...set frame size and burst length = 8 words (?) */
	value = (panel_info.vl_col * panel_info.vl_row *
		 NBITS(panel_info.vl_bpix)) / 32;
	value |= ((ATMEL_LCDC_DMA_BURST_LEN - 1) << ATMEL_LCDC_BLENGTH_OFFSET);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMAFRMCFG, value);

	/* Set pixel clock */
	value = get_lcdc_clk_rate(0) / panel_info.vl_clk;
	if (get_lcdc_clk_rate(0) % panel_info.vl_clk)
		value++;
	value = (value / 2) - 1;

	if (!value) {
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON1, ATMEL_LCDC_BYPASS);
	} else
		lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON1,
			    value << ATMEL_LCDC_CLKVAL_OFFSET);

	/* Initialize control register 2 */
#ifdef CONFIG_AVR32
	value = ATMEL_LCDC_MEMOR_BIG | ATMEL_LCDC_CLKMOD_ALWAYSACTIVE;
#else
	value = ATMEL_LCDC_MEMOR_LITTLE | ATMEL_LCDC_CLKMOD_ALWAYSACTIVE;
#endif
	if (panel_info.vl_tft)
		value |= ATMEL_LCDC_DISTYPE_TFT;

	value |= panel_info.vl_sync;
	value |= (panel_info.vl_bpix << 5);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDCON2, value);

	/* Vertical timing */
	value = (panel_info.vl_vsync_len - 1) << ATMEL_LCDC_VPW_OFFSET;
	value |= panel_info.vl_upper_margin << ATMEL_LCDC_VBP_OFFSET;
	value |= panel_info.vl_lower_margin;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_TIM1, value);

	/* Horizontal timing */
	value = (panel_info.vl_right_margin - 1) << ATMEL_LCDC_HFP_OFFSET;
	value |= (panel_info.vl_hsync_len - 1) << ATMEL_LCDC_HPW_OFFSET;
	value |= (panel_info.vl_left_margin - 1);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_TIM2, value);

	/* Display size */
	value = (panel_info.vl_col - 1) << ATMEL_LCDC_HOZVAL_OFFSET;
	value |= panel_info.vl_row - 1;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_LCDFRMCFG, value);

	/* FIFO Threshold: Use formula from data sheet */
	value = ATMEL_LCDC_FIFO_SIZE - (2 * ATMEL_LCDC_DMA_BURST_LEN + 3);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_FIFO, value);

	/* Toggle LCD_MODE every frame */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_MVAL, 0);

	/* Disable all interrupts */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_IDR, ~0UL);

	/* Set contrast */
	value = ATMEL_LCDC_PS_DIV8 |
		ATMEL_LCDC_POL_POSITIVE |
		ATMEL_LCDC_ENA_PWMENABLE;
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_CONTRAST_CTR, value);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_CONTRAST_VAL, ATMEL_LCDC_CVAL_DEFAULT);

	/* Set framebuffer DMA base address and pixel offset */
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMABADDR1, (u_long)lcdbase);

	lcdc_writel(panel_info.mmio, ATMEL_LCDC_DMACON, ATMEL_LCDC_DMAEN);
	lcdc_writel(panel_info.mmio, ATMEL_LCDC_PWRCON,
		    (ATMEL_LCDC_GUARD_TIME << ATMEL_LCDC_GUARDT_OFFSET) | ATMEL_LCDC_PWR);
}

ulong calc_fbsize(void)
{
	return ((panel_info.vl_col * panel_info.vl_row *
		NBITS(panel_info.vl_bpix)) / 8) + PAGE_SIZE;
}
