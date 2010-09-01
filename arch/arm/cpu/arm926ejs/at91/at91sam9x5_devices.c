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
#define CONFIG_USART3		1		/* USART 3 is DBGU */
#include <../drivers/serial/atmel_usart.h>

#define USART_BASE	USART3_BASE
#define AT91C_PIO_PA9         (1 << 9) /* Pin Controlled by PA9 */
#define AT91C_PA9_DRXD        (AT91C_PIO_PA9)
#define AT91C_PIO_PA10        (1 << 10) /* Pin Controlled by PA10 */
#define AT91C_PA10_DTXD       (AT91C_PIO_PA10)

unsigned int SetBaudrate (
	const unsigned int master_clock, 	// Peripheral Clock
	const unsigned int baud_rate)  		// UART Baudrate
{
	unsigned int baud_value = ((master_clock*10)/(baud_rate * 16));
	if ((baud_value % 10) >= 5)
		baud_value = (baud_value / 10) + 1;
	else
		baud_value /= 10;
	return baud_value;
}

void at91_serial3_hw_init(void)
{
	
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;
	writel(1 << AT91SAM9X5_ID_PIOAB, &pmc->pcer);
	writel(1 << AT91SAM9X5_ID_SYS, &pmc->pcer);
	
	unsigned int baud;

	// Reset receiver.
	usart3_writel(CR, USART3_BIT(RSTRX));
	usart3_writel(CR, USART3_BIT(RXEN)); 
    
    	// Disable interrupts
	usart3_writel(IDR, 0xffffffff);
    	usart3_writel(CR, USART3_BIT(RSTRX) | USART3_BIT(RSTTX) | USART3_BIT(RXDIS) | USART3_BIT(TXDIS));
    
    	// Set Baudrate    
    	baud = SetBaudrate(133000000, 115200);
	usart3_writel(BRGR, baud);

    	// MUX PIO periph A
    	at91_set_a_periph(AT91_PIO_PORTA, 9, 0);	/* DRXD */
	at91_set_a_periph(AT91_PIO_PORTA, 10, 1);	/* DTXD */

	at91_port_t  *pioa	= (at91_port_t *) (0xFFFFF400);
	pioa->pdr = AT91C_PA9_DRXD|AT91C_PA10_DTXD;
	
	// Enable TX + RX
	usart3_writel(CR, USART3_BIT(TXEN));
	usart3_writel(CR, USART3_BIT(RXEN));

}

void blink(int times)
{
	at91_port_t  *pioc	= (at91_port_t *) (0xfffff800);
	volatile unsigned int i;
	while (times--)
	{
		// Clear the LED's. On the board we must apply a "1" to turn off LEDs
		for (i=0 ; i<1000000;i++)
		{
			at91_set_pio_value(AT91_PIO_PORTC, 19, 1);
			at91_set_pio_value(AT91_PIO_PORTC, 20, 1);
			at91_set_pio_value(AT91_PIO_PORTC, 21, 1);
			at91_set_pio_value(AT91_PIO_PORTC, 25, 1);
		}		

		// Clear the LED's. On the board we must apply a "0" to turn on LEDs		
		for (i=0 ; i<1000000;i++)
		{
			at91_set_pio_value(AT91_PIO_PORTC, 19, 0);
			at91_set_pio_value(AT91_PIO_PORTC, 20, 0);
			at91_set_pio_value(AT91_PIO_PORTC, 21, 0);
			at91_set_pio_value(AT91_PIO_PORTC, 25, 0);
		}
	}
}

void at91_led_hw_init(void)
{
	at91_pmc_t	*pmc	= (at91_pmc_t *) AT91_PMC_BASE;
	writel(1 << AT91SAM9X5_ID_PIOCD, &pmc->pcer);
	
	/* PC19: YELLOW LED.
	   PC20: GREEN  LED.
	   PC21: BLUE   LED.
	   PC25: RED power LED. */
	at91_set_c_periph(AT91_PIO_PORTC, 19, 1);
	at91_set_c_periph(AT91_PIO_PORTC, 20, 1);
	at91_set_c_periph(AT91_PIO_PORTC, 21, 1);
	at91_set_c_periph(AT91_PIO_PORTC, 25, 1);
	
	at91_set_pio_output(AT91_PIO_PORTC, 19, 0);
	at91_set_pio_output(AT91_PIO_PORTC, 20, 0);
	at91_set_pio_output(AT91_PIO_PORTC, 21, 0);
	at91_set_pio_output(AT91_PIO_PORTC, 25, 0);

}

void at91_serial_hw_init(void)
{
	at91_led_hw_init();

#ifdef CONFIG_USART3	/* DBGU */
	at91_serial3_hw_init();
#endif

	printf ("%s\n", "Turn on all the LEDs.");
}

#ifdef CONFIG_MACB
void at91_macb_hw_init(void)
{
	at91_set_a_periph(AT91_PIO_PORTB, 4, 0);	/* ETXCK_EREFCK */
	at91_set_a_periph(AT91_PIO_PORTB, 3, 0);	/* ERXDV */
	at91_set_a_periph(AT91_PIO_PORTB, 0, 0);	/* ERX0 */
	at91_set_a_periph(AT91_PIO_PORTB, 1, 0);	/* ERX1 */
	at91_set_a_periph(AT91_PIO_PORTB, 2, 0);	/* ERXER */
	at91_set_a_periph(AT91_PIO_PORTB, 7, 0);	/* ETXEN */
	at91_set_a_periph(AT91_PIO_PORTB, 9, 0);	/* ETX0 */
	at91_set_a_periph(AT91_PIO_PORTB, 10, 0);	/* ETX1 */
	at91_set_a_periph(AT91_PIO_PORTB, 5, 0);	/* EMDIO */
	at91_set_a_periph(AT91_PIO_PORTB, 6, 0);	/* EMDC */
#ifndef CONFIG_RMII
	at91_set_b_periph(AT91_PIO_PORTB, 16, 0);	/* ECRS */
	at91_set_b_periph(AT91_PIO_PORTB, 17, 0);	/* ECOL */
	at91_set_b_periph(AT91_PIO_PORTB, 13,  0);	/* ERX2 */
	at91_set_b_periph(AT91_PIO_PORTB, 14,  0);	/* ERX3 */
	at91_set_b_periph(AT91_PIO_PORTB, 15, 0);	/* ERXCK */
	at91_set_b_periph(AT91_PIO_PORTB, 11,  0);	/* ETX2 */
	at91_set_b_periph(AT91_PIO_PORTB, 12,  0);	/* ETX3 */
	at91_set_b_periph(AT91_PIO_PORTB, 8, 0);	/* ETXER */
#endif
}
#endif
