/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include "core_cm4.h"

/* Constants specifying delay loop duration */
#define tdelay1 32768
#define tdelay2 	10

/* Auxiliary global variable */
//int counter = 0;

/* Variable delay loop */
void delay(int t1, int t2)
{
	int i, j;

	for(i=0; i<t1; i++) {
		for(j=0; j<t2; j++);
	}
}

// PORTA
#define A0 8
#define A1 10
#define A2 6
#define A3 11

// PORTE
#define EN 28

// PORTA
#define R0 26
#define R1 24
#define R2 9
#define R3 25
#define R4 28
#define R5 7
#define R6 27
#define R7 29

// PORTE
#define SW2 10
#define SW3 12
#define SW4 27
#define SW5 26
#define SW6 11

// PORTB
#define D9  5
#define D10 4
#define D11 3
#define D12 2


#define COL_MASK 0x00000d40
#define ROW_MASK 0x3f000280
#define EN_MASK  0x10000000

#define LED_MASK 0x0000003C

const unsigned int ROW_ARR[8] = { R0, R1, R2, R3, R4, R5, R6, R7 };

// 16 * 8 = 128
unsigned int field[128] = {
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,

		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0,
		0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0
};


enum direction {
	STOP = -1, UP = 0, LEFT, DOWN, RIGHT
};

typedef struct {
	unsigned int x;
	unsigned int y;
} pos_t;

#define SNAKE_LEN 6
#define SNAKE_DISPLAY_LOOP_DELAY 1024
#define SNAKE_SPEED 8

int snake_dir = STOP;

pos_t snake_body[SNAKE_LEN] = {
		{ 3, 9  }, // Head
		{ 3, 10 },
		{ 3, 11 },
		{ 3, 12 },
		{ 3, 13 },
		{ 3, 14 },
		{ 3, 15 } // Tail
};



/*
 *	This method configures the Multipurpose Clock Generator (MCG) to run in
 *	Bypassed Low Power External (BLPE) mode from FEI mode. On the TWR-K60N512
 *	Bypassed board, this would be 50 MHz.
 *
 */
void MCG_FEI_BLPE() {
	// Select External Reference Clock in MCG (Transition to FBE from FEI)
	// C1[CLKS] written to 10
	MCG->C1 = MCG_C1_CLKS(2);

	// Wait for Reference clock to switch to external reference
	while (MCG->S & MCG_S_IREFST_MASK);

	// Wait for MCGOUT to switch over to the external reference clock
	while (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x2);

	// Transition to bypassed low power external mode (BLPE)
	// C2[LP] written to 1
	MCG->C2 = MCG_C2_LP_MASK;
}

/*
 *	This method configures the Multipurpose Clock Generator (MCG) to run in
 *	FLL Engaged Internal (FEI) mode from BLPE mode. For K60 0M33Z masks, this
 *	is 25 MHz, otherwise it is 20.97 MHz.
 *
 */
void MCG_BLPE_FEI() {
	// Transition to FBE
	MCG->C2 = 0;

	// Transition to FEI
	MCG->C1 = MCG_C1_IREFS_MASK;

	// Wait for Reference clock to switch to internal reference
	while ((MCG->S & MCG_S_IREFST_MASK) == 0);

	// Wait for MCGOUT to switch over to the FLL clock
	while (((MCG->S & MCG_S_CLKST_MASK) >> MCG_S_CLKST_SHIFT) != 0x0);
}


/*
 * Port E ISR
 */
void PORTE_IRQHandler() {
	interrupt_button_handler();
	// Clear all Port E interrupt flags
	PORTE->ISFR = PORT_ISFR_ISF_MASK;
}

void PIT0_IRQHandler() {
	// Clear the Interrupt Flag in the PIT module
	PIT->CHANNEL[0].TFLG = 0x01;

	// Toggle the LED
	PTB->PTOR = (1 << 5);

	if (snake_dir != STOP) {
		update_snake_pos();
	}
}






void init_led(void)
{
	/* Set corresponding PTB pins (connected to LED's) for GPIO functionality */
	PORTB->PCR[D9]  = ( PORT_PCR_MUX(0x01) ); // D9
	PORTB->PCR[D10] = ( PORT_PCR_MUX(0x01) ); // D10
	PORTB->PCR[D11] = ( PORT_PCR_MUX(0x01) ); // D11
	PORTB->PCR[D12] = ( PORT_PCR_MUX(0x01) ); // D12

	/* Change corresponding PTB port pins as outputs */
	PTB->PDDR  = GPIO_PDDR_PDD( LED_MASK ); //0x3C
	PTB->PDOR |= GPIO_PDOR_PDO( LED_MASK );
}

void init_buttons(void)
{
	PORTE->PCR[SW2] = PORT_PCR_MUX(0x01); // SW2
	PORTE->PCR[SW3] = PORT_PCR_MUX(0x01); // SW3
	PORTE->PCR[SW4] = PORT_PCR_MUX(0x01); // SW4
	PORTE->PCR[SW5] = PORT_PCR_MUX(0x01); // SW5
	PORTE->PCR[SW6] = PORT_PCR_MUX(0x01); // SW6
}

void init_display(void)
{
	// Set display EN to output
	PORTE->PCR[EN] = ( PORT_PCR_MUX(0x01) );
	// Set EN to 0 (MEANS ON) !!!
	PTE->PDDR = GPIO_PDOR_PDO( EN_MASK );
	PTE->PDOR &= ~( GPIO_PDOR_PDO( EN_MASK) );

	// Display COL mux pins
	// A0 -> 8, A1 -> 10, A2 -> 6, A3 -> 11
	PORTA->PCR[A0] = ( PORT_PCR_MUX(0x01) ); // A0
	PORTA->PCR[A1] = ( PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[A2] = ( PORT_PCR_MUX(0x01) ); // A2
	PORTA->PCR[A3] = ( PORT_PCR_MUX(0x01) ); // A3

	// Set COL mux pins as outputs
	PTA->PDDR = GPIO_PDDR_PDD( COL_MASK | ROW_MASK ); // 0x0000  0000 1101  0100 0000
	// 0000 -> pin0, 1111 -> pin15
	//PTA->PDOR |= GPIO_PDOR_PDO( 0xD40 );
	//PTA->PDOR &= ~( GPIO_PDOR_PDO( col_mask ) );
	PTA->PDOR |= GPIO_PDOR_PDO( COL_MASK ); // set to 1111


	// Set display ROW pins to output
	// R0 -> 26, R1 -> 24, R2 -> 9, R3 -> 25, R4 -> 28, R5 -> 7, R6 -> 27, R7 -> 29
	PORTA->PCR[R0] = ( PORT_PCR_MUX(0x01) ); // R0
	PORTA->PCR[R1] = ( PORT_PCR_MUX(0x01) ); // R1
	PORTA->PCR[R2] = ( PORT_PCR_MUX(0x01) ); // R2
	PORTA->PCR[R3] = ( PORT_PCR_MUX(0x01) ); // R3
	PORTA->PCR[R4] = ( PORT_PCR_MUX(0x01) ); // R4
	PORTA->PCR[R5] = ( PORT_PCR_MUX(0x01) ); // R5
	PORTA->PCR[R6] = ( PORT_PCR_MUX(0x01) ); // R6
	PORTA->PCR[R7] = ( PORT_PCR_MUX(0x01) ); // R7

	// Set ROW pins as outputs
	//PTA->PDDR  = GPIO_PDDR_PDD( row_mask ); // 0x0000  0000 1101  0100 0000

	// Set ROW 0 to ON
	PTA->PDOR |= GPIO_PDOR_PDO( 1 << R0 );
	PTA->PDOR |= GPIO_PDOR_PDO( 1 << R1 );
	PTA->PDOR |= GPIO_PDOR_PDO( 1 << R2 );

	// Turn off (1 = ON, 0 = OFF)
	//PTA->PDOR &= ~( GPIO_PDOR_PDO( row_mask ) );
}

void init_pit(void)
{
	// Configure the Signal Multiplexer for SW6, configure SW6 to interrupt on falling edge
	PORTE->PCR[SW2] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW3] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW4] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW5] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);

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
	PIT->CHANNEL[0].LDVAL = 24999999 / SNAKE_SPEED;

	// Enable Interrupt and Start Timer
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	//PIT->CHANNEL[1].LDVAL = 24999999 / 10000;

	// Enable Interrupt and Start Timer
	//PIT->CHANNEL[1].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT0 Interrupt Channel in NVIC
	NVIC->ISER[2] |= (1 << 4);
	// Enable PIT1 Interrupt Channel in NVIC
	//NVIC->ISER[2] |= (1 << 5);
}



void play_led_sequence(void)
{
	//Turn all the LEDs sequentialy ON!
    PTB->PDOR &= ~(1 << 5);
    delay(tdelay1, tdelay2);
    PTB->PDOR &= ~(1 << 4);
    delay(tdelay1, tdelay2);
    PTB->PDOR &= ~(1 << 3);
    delay(tdelay1, tdelay2);
    PTB->PDOR &= ~(1 << 2);
    delay(tdelay1, tdelay2);

    //Turn all the LEDs sequentialy OFF!
   	PTB->PDOR |= (1 << 2);
   	delay(tdelay1, tdelay2);
   	PTB->PDOR |= (1 << 3);
   	delay(tdelay1, tdelay2);
   	PTB->PDOR |= (1 << 4);
   	delay(tdelay1, tdelay2);
   	PTB->PDOR |= (1 << 5);
   	delay(tdelay1, tdelay2);
}




void interrupt_button_handler(void)
{
	if ( (PORTE->ISFR & (1 << SW2)) != 0 ) {
		PTB->PCOR = 1 << D9;
		snake_dir = UP;
	} else {
		PTB->PSOR = 1 << D9;
	}

	if ( (PORTE->ISFR & (1 << SW3)) != 0 ) {
		PTB->PCOR = 1 << D10;
		snake_dir = RIGHT;
	} else {
		PTB->PSOR = 1 << D10;
	}

	if ( (PORTE->ISFR & (1 << SW4)) != 0 ) {
		PTB->PCOR = 1 << D11;
		snake_dir = DOWN;
	} else {
		PTB->PSOR = 1 << D11;
	}

	if ( (PORTE->ISFR & (1 << SW5)) != 0 ) {
		PTB->PCOR = 1 << D12;
		snake_dir = LEFT;
	} else {
		PTB->PSOR = 1 << D12;
	}
}


void set_col_on(unsigned int col_id)
{
	// Set all cols OFF
	PTA->PCOR = COL_MASK;
	// Extract the individual bits from col_id and shift to the cols positions to create a mask
	PTA->PSOR = ((col_id	 ) & 1u) << A0
			  | ((col_id >> 1) & 1u) << A1
			  | ((col_id >> 2) & 1u) << A2
			  | ((col_id >> 3) & 1u) << A3;
}

void set_row_on(unsigned int row_id)
{
	// Set all rows OFF
	PTA->PCOR = ROW_MASK;
	// Set the current row ON
	PTA->PSOR = 1 << ROW_ARR[row_id];
}


void display_snake(void)
{
	for (int i = SNAKE_LEN-1; i >=0; --i) {
		delay(SNAKE_DISPLAY_LOOP_DELAY, 1);

		set_row_on(snake_body[i].x);
		set_col_on(snake_body[i].y);
	}
}

void update_snake_pos(void)
{
	pos_t new_head_pos = { snake_body[0].x, snake_body[0].y };
	switch (snake_dir) {
	case UP: // Row 0 is the upper one
		--new_head_pos.x;
		break;
	case LEFT:
		--new_head_pos.y;
		break;
	case DOWN:
		++new_head_pos.x;
		break;
	case RIGHT:
		++new_head_pos.y;
		break;
	default:
		break;
	}

	new_head_pos.x %= 8;
	new_head_pos.y %= 16;

	for (int i = SNAKE_LEN-1; i > 0; --i) {
		snake_body[i] = snake_body[i-1];
		snake_body[i].x %= 8;
		snake_body[i].y %= 16;
	}

	snake_body[0] = new_head_pos;
}



int main(void)
{
	/* Turn on all port clocks */
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;

	init_led();
	init_buttons();
	init_display();
	init_pit();

	play_led_sequence();

	snake_dir = STOP;
	//unsigned int snake_delay = tdelay1 * tdelay2;
	for (;;) {
		display_snake();
    }
    /* Never leave main */
    return 0;
}
