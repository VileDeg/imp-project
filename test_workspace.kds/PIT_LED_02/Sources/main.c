/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include "core_cm4.h"
#include "mcg.h"

/*
 * Port E ISR
 */
void PORTE_IRQHandler() {
	// Clear all Port E interrupt flag
	PORTE->ISFR = PORT_ISFR_ISF_MASK;

	// Toggle the LED
	PTB->PTOR = (1 << 2);
}

void PIT0_IRQHandler() {
	// Clear the Interrupt Flag in the PIT module
	PIT->CHANNEL[0].TFLG = 0x01;

	// Toggle the LED
	PTB->PTOR = (1 << 5);
}


int main(void) {
	unsigned int x = 0, y = 0; // Counter variables

	// Configure the MCG to run in BLPE mode. (50 MHz)
	MCG_FEI_BLPE();

	// Enable clocks on Port B
	SIM->SCGC5 = SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;

	/* Configure the Signal Multiplexer for the LEDs (GPCLR is a method to set multiple pins simultaneously)
	 *                   D12        D9
	 */
	PORTB->GPCLR = ( ((1 << 2) | (1 << 5)) << 16) | PORT_PCR_MUX(0x01);

	/* Set corresponding PTB port pins as outputs */
	PTB->PDDR = (1 << 2) | (1 << 5);
	PTB->PSOR = (1 << 2);
	PTB->PCOR = (1 << 5);

	// Configure the Signal Multiplexer for SW6, configure SW6 to interrupt on falling edge
	PORTE->PCR[11] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010); // SW6

	// Clear the Interrupt flags in Port E
	PORTE->ISFR = 0xFFFFFFFF;

	// Clear the Interrupt pending flag in the NVIC just in case
	NVIC->ICPR[2] = (1 << 27);

	// Enable the Interrupt in the NVIC
	NVIC->ISER[2] = (1 << 27);

	// Enable the clock on the PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

	// Turn on the PIT
	PIT->MCR = 0x00;

	// Load Timer Value into Channel 0
	// LDVAL trigger = (period/clock period) - 1
	// LDVAL trigger = (0.5 / (1 / 50,000,000)) - 1
	PIT->CHANNEL[0].LDVAL = 24999999;

	// Enable Interrupt and Start Timer
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT0 Interrupt Channel in NVIC
	NVIC->ISER[2] |= (1 << 4);

	for(;;) {
		// Do nothing. There should be no code here in this Lab.
	}

	return 0;
}
