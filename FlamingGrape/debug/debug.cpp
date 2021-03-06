#include "debug/debug.h"
#include "hw/uart.h"
#include "hw/adc.h"
#include "hw/motor.h"
#include "hw/tick.h"
#include "util.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// debug LED
static PORT_t &ledport = PORTR;
static const int boardled_mask = _BV(1);

// debug LED BoB
static PORT_t &debugledport = PORTE;
static const int debugled_mask = _BV(2) | _BV(3) | _BV(4) | _BV(5);

// debug timer
static TC1_t &tim = TCC1;

// output flags
static bool echo_enabled = true;

// battery monitor
static uint8_t batterycnt;

// stdio stuff
static int put(char ch, FILE* file);
static int get(FILE* file);
static FILE stdinout;

void debug_init() {
	ledport.DIRSET = boardled_mask;
	debugledport.DIRSET = debugled_mask;
	debug_setLED(BOARD_LED, false);
	
	tim.CTRLA = TC_CLKSEL_DIV64_gc; // 32Mhz / 64 = .5 Mhz timer
	tim.PER = 0xFFFF; // 1Mhz / 65536 = 65ms
	
	fdev_setup_stream(&stdinout, put, get, _FDEV_SETUP_RW);
	stdin = &stdinout;
	stdout = &stdinout;
}

void debug_tick() {
	if (adc_getBattery() < 10.5) {
		if (++batterycnt >= 100)
			debug_halt("STOP USING THE ROBOT CHARGE ME BRO");
	} else {
		batterycnt = 0;
	}
}

static int put(char ch, FILE* file) {
	if(ch == '\n')
		put('\r', file);
		
	while (!uart_put(UART_USB, ch)) { }
	while (!uart_put(UART_XBEE, ch)) { }
	
	return 1;
}

static int get(FILE* file) {
	int ch;
	do {
		ch = uart_get(UART_USB);
		if (ch == -1)
			ch = uart_get(UART_XBEE);
	} while (ch == -1);
	
	if (ch == '\r')
		ch = '\n';
	
	if (echo_enabled)
		put(ch, NULL); // echo character
	
	return ch;
}

void debug_setLED(DebugLED led, bool on) {
	if (led == BOARD_LED) {
		if (on) {
			ledport.OUTCLR = boardled_mask;
		} else {
			ledport.OUTSET = boardled_mask;
		}
	} else {
		if (on) {
			debugledport.OUTSET = _BV(led + 1);
		} else {
			debugledport.OUTCLR = _BV(led + 1);
		}
	}
}

void debug_resetTimer() {
	tim.CNT = 0;
}

uint16_t debug_getTimer() {
	return tim.CNT * 2;
}

void debug_println(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	char buf[64];
	unsigned int amt = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (amt > sizeof(buf)-4)
		amt = sizeof(buf)-4;
	buf[amt++] = '\r';
	buf[amt++] = '\n';
	buf[amt++] = 0;
	
	uart_puts(UART_USB, buf); // no resending logic, we drop bytes if buffer fills up
	
	va_end(ap);
}

void debug_setEchoEnabled(bool enabled) {
	echo_enabled = enabled;
}

void debug_halt(const char *reason) {
	tick_suspend();
	motor_allOff();
	
	bool led=false;
	while (true) {
		printf_P(PSTR("Halted like a boss. %s\n"), reason);
		debug_setLED(BOARD_LED, led);
		led = !led;
		_delay_ms(1000);
	}
}
