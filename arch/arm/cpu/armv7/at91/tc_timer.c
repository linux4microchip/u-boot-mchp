/*
 * (C) Copyright 2013 Atmel
 * 		 2013 Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/atmel_tc.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <div64.h>

#if !defined(CONFIG_AT91FAMILY)
# error You need to define CONFIG_AT91FAMILY in your board config!
#endif

DECLARE_GLOBAL_DATA_PTR;

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, gd->arch.timer_rate_hz);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= gd->arch.timer_rate_hz;
	do_div(usec, 1000000);

	return usec;
}

/*
 * Use the tcb in full 32 bit incrementing mode
 */
int timer_init(void)
{
	struct atmel_tc *tc = (struct atmel_tc *) ATMEL_BASE_TC0;

	/* Enable TC Clock */
	at91_periph_clk_enable(ATMEL_ID_TC0);

	writel(0x01 | ATMEL_TC_WAVE | ATMEL_TC_WAVESEL_UP, &tc->cmr);
	writel(0xff, &tc->idr);
	writel(ATMEL_TC_CLKEN, &tc->ccr);

	writel(ATMEL_TC_SYNC, &tc->bcr);
	gd->arch.timer_rate_hz = gd->arch.mck_rate_hz / 16;
	gd->arch.tbu = 0;
	gd->arch.tbl = 0;

	return 0;
}

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	struct atmel_tc *tc = (struct atmel_tc *) ATMEL_BASE_TC0;

	ulong now = readl(&tc->cv);
	if (readl(&tc->sr) & 0x1)
		gd->arch.tbu++;
	gd->arch.tbl = now;
	return (((unsigned long long)gd->arch.tbu) << 32) | gd->arch.tbl;
}

void __udelay(unsigned long usec)
{
	unsigned long long start;
	ulong tmo;

	start = get_ticks();		/* get current timestamp */
	tmo = usec_to_tick(usec);	/* convert usecs to ticks */
	while ((get_ticks() - start) < tmo)
		;			/* loop till time has passed */
}

/*
 * get_timer(base) can be used to check for timeouts or
 * to measure elasped time relative to an event:
 *
 * ulong start_time = get_timer(0) sets start_time to the current
 * time value.
 * get_timer(start_time) returns the time elapsed since then.
 *
 * The time is used in CONFIG_SYS_HZ units!
 */
ulong get_timer(ulong base)
{
	return tick_to_time(get_ticks()) - base;
}

/*
 * Return the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->arch.timer_rate_hz;
}
