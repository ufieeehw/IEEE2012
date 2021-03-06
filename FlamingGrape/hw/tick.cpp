#include "tick.h"
#include <avr/io.h>
#include <avr/interrupt.h>

static TC1_t &tim = TCF1;
#define TIMOVFVEC TCF1_OVF_vect
#define TIMCCAVEC TCF1_CCA_vect

static volatile uint32_t tickcount;
static volatile uint16_t ticklength;

void tick_init() {
	tim.CTRLA = TC_CLKSEL_DIV8_gc; // 32Mhz / 8 = 4 Mhz timer (TICK_TIMHZ == 4E6)
	tim.CTRLB = TC0_CCAEN_bm; // enable capture compare A
	tim.PER = TICK_TIMMAX; // TICK_TIMHZ / (TICK_TIMHZ / TICK_HZ) = TICK_HZ timer
	tim.CCABUF = 200; // 200 / 4Mhz = 50us CCA (for linesensor)
	tick_resume(); // enable interrupts
}

void tick_wait() {
	uint32_t t = tickcount;
	while (t == tickcount) { }
}

void tick_suspend() {
	tim.INTCTRLA = 0;
	tim.INTCTRLB = 0;
}
void tick_resume() {
	tim.INTCTRLB = TC_CCAINTLVL_HI_gc; // capture compare A interrupt enabled at high priority, because the tick may not be completed before this goes off
	tim.INTCTRLA = TC_OVFINTLVL_LO_gc; // overflow interrupt enabled at low priority, for running the ticks
}

uint16_t tick_getTimer() {
	return tim.CNT;
}

uint32_t tick_getCount() {
	return tickcount;
}

uint16_t tick_getLength() { // in uS
	return (uint16_t)((uint32_t)ticklength * TICK_US / TICK_TIMMAX);
}

#include "competition/sensorcomms.h"
#include "control/motorcontrol.h"
#include "control/linefollow.h"
#include "control/magfollow.h"
#include "control/traj.h"
#include "control/deploy.h"
#include "debug/debug.h"
#include "hw/linesensor.h"
#include "hw/startbutton.h"
#include "debug/debug.h"
#include <util/delay.h>

ISR(TIMOVFVEC) {
	uint16_t start = tim.CNT;
	tickcount++;
	debug_setLED(BOARD_LED, true);
	
	linesensor_tick();
	linefollow_tick();
	magfollow_tick();
	traj_tick();
	motorcontrol_tick();
	debug_tick();
	deploy_tick();
	sensorcomms_tick();
	startbutton_tick();

	debug_setLED(BOARD_LED, false);
	ticklength = tim.CNT - start;
}

ISR(TIMCCAVEC) {
	linesensor_tick50us();
}

