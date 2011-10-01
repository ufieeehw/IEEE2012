#include <stdio.h>
#include <string.h>

#include <avr/interrupt.h>
#include <avr/io.h>

#include "util.h"

#include "debug.h"

// debug LED
static PORT_t &ledport = PORTR;
static const int ledpin = 1;

// debug/USB uart
static USART_t &uart = USARTC0;
#define RXVEC USARTC0_RXC_vect
static PORT_t &uartport = PORTC;
static const int txpin = 3;
static const int rxpin = 2;
static const int bsel = 2158; // makes 115200 baud
static const int bscale = 0xA;

static char recvbuf[8];
static volatile uint8_t recvbuf_pos;

void debug_init() {
	ledport.DIRSET = _BV(ledpin);
	debug_setLED(false);
	
	uartport.OUTSET = _BV(txpin); // make pin high to avoid transmitting a false start bit on startup
	uartport.DIRSET = _BV(txpin);
	uart.CTRLA = USART_RXCINTLVL_LO_gc;
	uart.CTRLB = USART_RXEN_bm | USART_TXEN_bm | USART_CLK2X_bm;
	uart.CTRLC = USART_CHSIZE_8BIT_gc;
	uart.BAUDCTRLA = bsel & 0xFF;
	uart.BAUDCTRLB = (bscale << USART_BSCALE_gp) | (bsel >> 8);
}

void debug_setLED(bool on) {
	if (on)
		ledport.OUTCLR = _BV(ledpin);
	else
		ledport.OUTSET = _BV(ledpin);
}

void debug_putch(char ch) {
	while (!(uart.STATUS & USART_DREIF_bm)) { }
	uart.DATA = ch;
}

void debug_puts(const char *s) {
	while (*s)
		debug_putch(*s++);
}

void debug_printf(const char *fmt, ...) {
	static char buf[64];
	
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	debug_puts(buf);
	va_end(ap);
}

char debug_getch() {
	while (recvbuf_pos == 0) { }
	
	util_cli_lo();
	char ch = recvbuf[0];
	memmove(recvbuf, recvbuf+1, recvbuf_pos - 1);
	recvbuf_pos--;
	util_sei_lo();
	
	return ch;
}

size_t debug_gets(char *buf, size_t len) {
	size_t amt=0;
	
	while (len) {
		char ch = debug_getch();
		if (ch == '\n')
			break;
		
		*buf++ = ch;
		amt++;
		len--;
	}
	
	return amt;
}

ISR(RXVEC) {
	uint8_t ch = uart.DATA;
	if (recvbuf_pos >= sizeof(recvbuf))
		return;
	
	recvbuf[recvbuf_pos++] = ch;
}
