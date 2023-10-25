/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"

/* Constants specifying delay loop duration */
#define tdelay1 32768
#define tdelay2 	10

/* Auxiliary global variable */
int counter = 0;

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

const unsigned int ROW_ARR[8] = { R0, R1, R2, R3, R4, R5, R6, R7 };

#define COL_MASK 0x00000d40
#define ROW_MASK 0x3f000280
#define EN_MASK  0x10000000

#define LED_MASK 0x0000003C

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

//unsigned int field[128] = {
//		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
//		0,0,0,0, 1,1,1,0, 1,1,1,0, 0,0,0,0,
//		0,0,0,0, 1,0,1,0, 1,0,1,0, 0,0,0,0,
//		0,0,0,0, 1,0,1,0, 1,1,1,0, 0,0,0,0,
//
//		0,0,0,0, 0,0,0,0, 0,0,0,0, 1,0,0,0,
//		0,0,1,0, 0,1,0,1, 0,1,0,1, 0,1,0,0,
//		0,0,0,1, 1,0,0,0, 1,0,0,1, 1,1,0,0,
//		0,0,1,0, 0,1,0,1, 0,0,0,1, 0,1,0,0,
//};

enum direction {
	STOP = -1, UP = 0, LEFT, DOWN, RIGHT
};

typedef struct {
	unsigned int x;
	unsigned int y;
} pos_t;

#define SNAKE_LEN 5

int snake_dir = STOP;

pos_t snake_body[SNAKE_LEN] = {
		{ 3, 9  }, // Head
		{ 3, 10 },
		{ 3, 11 },
		{ 3, 12 },
		{ 3, 13 }, // Tail
};


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

void play_display_row_sequence(void)
{
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R0 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R1 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R2 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R3 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R4 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R5 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R6 );
	//delay(tdelay1, tdelay2);
	PTE->PDOR |= GPIO_PDOR_PDO( 1 << R7 );
}

void play_display_col_sequence(void)
{
	unsigned int COL_ARR[4] = { A0, A1, A2, A3 };
	unsigned int col_mask = (1 << A0) | (1 << A1) | (1 << A2) | (1 << A3);

	//for (unsigned int i = 0; i < 16; ++i) {
		for (unsigned int j = 0; j < 4; ++j) {
			//unsigned int bit_j = i & (1 << j);
			play_display_row_sequence();
			// Disable all others
			PTA->PDOR &= ~(GPIO_PDOR_PDO(col_mask));
			// Enable the current one
			PTA->PDOR |= GPIO_PDOR_PDO(1 << COL_ARR[j]);

			delay(tdelay1, tdelay2);
		}
	//}
}

void buttons_handler(void)
{
	/* Evaluation of SW2 state */
	if ( (PTE->PDIR & (1 << SW2)) != 0 ) {
		PTB->PDOR |= (1 << D9);
	} else {
		snake_dir = UP;
		PTB->PDOR &= ~(1 << D9);
	}

	/* Evaluation of SW3 state */
	if ( (PTE->PDIR & (1 << SW3)) != 0 ) {
		PTB->PDOR |= (1 << D10);
	} else {
		snake_dir = RIGHT;
		PTB->PDOR &= ~(1 << D10);
	}

	/* Evaluation of SW4 state */
	if ( (PTE->PDIR & (1 << SW4)) != 0 ) {
		PTB->PDOR |= (1 << D11);
	} else {
		snake_dir = DOWN;
		PTB->PDOR &= ~(1 << D11);
	}

	/* Evaluation of SW5 state */
	if ( (PTE->PDIR & (1 << SW5)) != 0 ) {
		PTB->PDOR |= (1 << D12);
	} else {
		snake_dir = LEFT;
		PTB->PDOR &= ~(1 << D12);
	}

	/* Evaluation of SW6 state */
	if ( (PTE->PDIR & (1 << SW6)) != 0 ) {
		PTB->PDOR |= GPIO_PDOR_PDO( LED_MASK );
	} else {
		PTB->PDOR &= ~GPIO_PDOR_PDO( LED_MASK );
	}
}

void init_display(void)
{
	//unsigned int col_mask = (1 << A0) | (1 << A1) | (1 << A2) | (1 << A3);
	//unsigned int row_mask = (1 << R0) | (1 << R1) | (1 << R2) | (1 << R3) | (1 << R4) | (1 << R5) | (1 << R6) | (1 << R7);
	//unsigned int en_mask  = 1 << EN; // 0001 0000 0x000000

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

void set_col_on(unsigned int col_id)
{
	// Leave ROWs untouched
	unsigned int pdor = PTA->PDOR & GPIO_PDOR_PDO( ROW_MASK );
	// Add all column bits
	pdor |= ((col_id >> 0) & 1u) << A0
		  | ((col_id >> 1) & 1u) << A1
		  | ((col_id >> 2) & 1u) << A2
		  | ((col_id >> 3) & 1u) << A3;

	// Set the final output
	PTA->PDOR = pdor;
}


void set_row_on(unsigned int row_id)
{
	// Set all rows OFF
	PTA->PDOR &= ~( GPIO_PDOR_PDO( ROW_MASK ) );

	PTA->PDOR |= GPIO_PDOR_PDO( 1 << ROW_ARR[row_id] );
	//PTA->PDOR &= GPIO_PDOR_PDO( 1 << ROW_ARR[row_id] );
}

void display_field(void)
{
//	for (int r = 0; r < 8; ++r){
//
//	}

	for (int i = 0; i < 128; ++i) {
		if (field[i] == 0) {
			continue;
		}
		//delay(tdelay1, tdelay2);
		int row = i / 16;
		int col = i % 16;

		set_row_on(row);
		set_col_on(col);
	}
}


void display_snake(void)
{
	for (int i = 0; i < SNAKE_LEN; ++i) {
		//delay(tdelay1, tdelay2);

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

	play_led_sequence();

	snake_dir = STOP;
	unsigned int snake_delay = tdelay1 * tdelay2;
	for (;;) {
		counter++;

		//delay(tdelay1, tdelay2);
		// Set all cols OFF
		//PTA->PDOR &= ~( GPIO_PDOR_PDO( COL_MASK ) );

//		if (counter % 2 == 0) {
//			set_col_on(0);
//		} else {
//			set_col_on(5);
//			//PTA->PDOR |= GPIO_PDOR_PDO( 1 << A1 ); // set to 0010
//			//PTA->PDOR |= GPIO_PDOR_PDO( 1 << A0 ); // set to 0001
//		}
		//display_field();



		if (counter > snake_delay) {
			counter = 0;

			buttons_handler();

			if (snake_dir >= UP && snake_dir <= RIGHT) {
				update_snake_pos();
			}
			display_snake();
		}




		//play_led_sequence();
		//play_display_row_sequence();
		//play_display_col_sequence();

		//handle_led_buttons();
    }
    /* Never leave main */
    return 0;
}
