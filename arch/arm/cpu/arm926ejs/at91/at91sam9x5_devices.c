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
#include <asm/arch/at91_common.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/gpio.h>
#include <asm/arch/io.h>

#include <asm/arch/memory-map.h>
#include <../drivers/serial/atmel_usart.h>

#define cpu_is_at91sam9x5()	(get_chip_id() == ARCH_ID_AT91SAM9X5)
#define cpu_is_at91sam9g15()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G15))
#define cpu_is_at91sam9g25()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G25))
#define cpu_is_at91sam9g35()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9G35))
#define cpu_is_at91sam9x25()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9X25))
#define cpu_is_at91sam9x35()	(cpu_is_at91sam9x5() && \
			(get_extension_chip_id() == ARCH_EXID_AT91SAM9X35))

unsigned int get_chip_id(void)
{
	return usart3_readl(CIDR);
}
unsigned int get_extension_chip_id(void)
{
	return usart3_readl(EXDR);
}

unsigned int has_emac1()
{
	return cpu_is_at91sam9x25();
}
unsigned int has_emac0()
{
	return !(cpu_is_at91sam9g15());
}
unsigned int has_lcdc()
{
	return cpu_is_at91sam9g15() || cpu_is_at91sam9g35()
		|| cpu_is_at91sam9x35();
}

char *get_cpu_name()
{
	unsigned int extension_id = get_extension_chip_id();
	if (cpu_is_at91sam9x5())
		switch (extension_id) {
		case ARCH_EXID_AT91SAM9G15:
			return CONFIG_SYS_AT91_G15_CPU_NAME;
		case ARCH_EXID_AT91SAM9G25:
			return CONFIG_SYS_AT91_G25_CPU_NAME;
		case ARCH_EXID_AT91SAM9G35:
			return CONFIG_SYS_AT91_G35_CPU_NAME;
		case ARCH_EXID_AT91SAM9X25:
			return CONFIG_SYS_AT91_X25_CPU_NAME;
		case ARCH_EXID_AT91SAM9X35:
			return CONFIG_SYS_AT91_X35_CPU_NAME;
		default:
			return CONFIG_SYS_AT91_UNKNOWN_CPU;
		}
	else
		return CONFIG_SYS_AT91_UNKNOWN_CPU;
}

unsigned int usart_set_baudrate(
	const unsigned int master_clock,	/* Peripheral Clock */
	const unsigned int baud_rate)		/* UART Baudrate */
{
	unsigned int baud_value = ((master_clock * 10) / (baud_rate * 16));
	if ((baud_value % 10) >= 5)
		baud_value = (baud_value / 10) + 1;
	else
		baud_value /= 10;
	return baud_value;
}

void at91_serial3_hw_init(void)
{
	unsigned int baud;
	at91_pmc_t *pmc = (at91_pmc_t *) AT91_PMC_BASE;
	writel(1 << AT91SAM9X5_ID_PIOAB, &pmc->pcer);
	writel(1 << AT91SAM9X5_ID_SYS, &pmc->pcer);

	/* Reset receiver */
	usart3_writel(CR, USART3_BIT(RSTRX));
	usart3_writel(CR, USART3_BIT(RXEN));
	/* Disable interrupts */
	usart3_writel(IDR, 0xffffffff);
	usart3_writel(CR, USART3_BIT(RSTRX) | USART3_BIT(RSTTX) \
		| USART3_BIT(RXDIS) | USART3_BIT(TXDIS));

	/* Set Baudrate */
	baud = usart_set_baudrate(133000000, 115200);
	usart3_writel(BRGR, baud);

	/* MUX PIO periph A */
	at91_set_a_periph(AT91_PIO_PORTA, 9, 0);	/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 1);	/* DTXD */

	at91_port_t *pioa = (at91_port_t *) (0xFFFFF400);
	pioa->pdr = AT91C_PA9_DRXD | AT91C_PA10_DTXD;

	/* Enable TX + RX */
	usart3_writel(CR, USART3_BIT(TXEN));
	usart3_writel(CR, USART3_BIT(RXEN));
}

void at91_serial_hw_init(void)
{
#ifdef CONFIG_USART3	/* DBGU */
	at91_serial3_hw_init();
#endif
}

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	if (has_emac0()) {
		at91_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* ETXCK */
		at91_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* ERXDV */
		at91_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ERX0 */
		at91_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* ERX1 */
		at91_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ERXER */
		at91_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ETXEN */
		at91_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ETX0 */
		at91_set_a_periph(AT91_PIO_PORTB, 10, 0);	/* ETX1 */
		at91_set_a_periph(AT91_PIO_PORTB, 5, 0);	/* EMDIO */
		at91_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* EMDC */
	}

	if (has_emac1()) {
		/* EMAC1 pins setup */
		at91_set_b_periph(AT91_PIO_PORTC, 29, 0);	/* ETXCK */
		at91_set_b_periph(AT91_PIO_PORTC, 28, 0);	/* ECRSDV */
		at91_set_b_periph(AT91_PIO_PORTC, 20, 0);	/* ERXO */
		at91_set_b_periph(AT91_PIO_PORTC, 21, 0);	/* ERX1 */
		at91_set_b_periph(AT91_PIO_PORTC, 16, 0);	/* ERXER */
		at91_set_b_periph(AT91_PIO_PORTC, 27, 0);	/* ETXEN */
		at91_set_b_periph(AT91_PIO_PORTC, 18, 0);	/* ETX0 */
		at91_set_b_periph(AT91_PIO_PORTC, 19, 0);	/* ETX1 */
		at91_set_b_periph(AT91_PIO_PORTC, 31, 0);	/* EMDIO */
		at91_set_b_periph(AT91_PIO_PORTC, 30, 0);	/* EMDC */
	}

#ifndef CONFIG_RMII
	/* Only emac0 support MII */
	if (has_emac0()) {
		at91_set_b_periph(AT91_PIO_PORTB, 16, 0);	/* ECRS */
		at91_set_b_periph(AT91_PIO_PORTB, 17, 0);	/* ECOL */
		at91_set_b_periph(AT91_PIO_PORTB, 13,  0);	/* ERX2 */
		at91_set_b_periph(AT91_PIO_PORTB, 14,  0);	/* ERX3 */
		at91_set_b_periph(AT91_PIO_PORTB, 15, 0);	/* ERXCK */
		at91_set_b_periph(AT91_PIO_PORTB, 11,  0);	/* ETX2 */
		at91_set_b_periph(AT91_PIO_PORTB, 12,  0);	/* ETX3 */
		at91_set_b_periph(AT91_PIO_PORTB, 8, 0);	/* ETXER */
	}
#endif
}
#endif
