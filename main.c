/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"
#include "core_cm4.h"

#include "stdlib.h"
#include "time.h"

typedef unsigned int uint;


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

#define LED_MASK  0x0000003C
#define BEEP_MASK 0x00002000


#define NUM_ROWS 8
#define NUM_COLS 16


uint ROW_ARR[NUM_ROWS] = { R0, R1, R2, R3, R4, R5, R6, R7 };

// 16 * 8 = 128
uint field[NUM_ROWS][NUM_COLS];

uint text_lost[NUM_ROWS][NUM_COLS] = {
		{ 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0 },
		{ 0,1,0,0, 1,1,1,1,  0,1,1,0, 1,1,1,0 },
		{ 0,1,0,0, 1,0,0,1,  0,1,0,0, 0,1,0,0 },
		{ 0,1,0,0, 1,0,0,1,  0,1,1,0, 0,1,0,0 },

		{ 0,1,0,0, 1,0,0,1,  0,0,1,0, 0,1,0,0 },
		{ 0,1,0,0, 1,0,0,1,  0,0,1,0, 0,1,0,0 },
		{ 0,1,1,0, 1,1,1,1,  0,1,1,0, 0,1,0,0 },
		{ 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0 },
};

uint text_start[NUM_ROWS][NUM_COLS] = {
		{ 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0 },
		{ 0,1,1,1, 1,1,1,0,  0,1,1,1, 1,1,1,0 },
		{ 0,1,1,1, 1,1,1,0,  0,1,1,1, 1,1,1,0 },
		{ 0,1,1,0, 0,0,0,0,  0,1,1,0, 0,1,1,0 },

		{ 0,1,1,0, 0,1,1,0,  0,1,1,0, 0,1,1,0 },
		{ 0,1,1,1, 1,1,1,0,  0,1,1,1, 1,1,1,0 },
		{ 0,1,1,1, 1,1,1,0,  0,1,1,1, 1,1,1,0 },
		{ 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0 },
};


enum direction {
	STOP = -1, UP = 0, LEFT, DOWN, RIGHT
};

typedef struct {
	uint x;
	uint y;
} pos_t;


#define GAME_DISPLAY_DELAY 1024

#define SNAKE_LEN_INIT 4
#define SNAKE_LEN_MAX 127

// Number of PIT ticks required to spawn a new food
#define FOOD_TICKS 20


int   snake_dir = STOP;
uint  snake_len = SNAKE_LEN_INIT;
pos_t snake_body[SNAKE_LEN_MAX];

uint  food_tick_counter = 0;


// Variable delay loop
void delay(uint t)
{
	for(uint i = 0; i < t; ++i);
}


void init_leds(void)
{
	// Set corresponding PTB pins (connected to LED's) for GPIO functionality
	PORTB->PCR[D9]  = ( PORT_PCR_MUX(0x01) ); // D9
	PORTB->PCR[D10] = ( PORT_PCR_MUX(0x01) ); // D10
	PORTB->PCR[D11] = ( PORT_PCR_MUX(0x01) ); // D11
	PORTB->PCR[D12] = ( PORT_PCR_MUX(0x01) ); // D12

	// Change corresponding PTB port pins as outputs
	PTB->PDDR  = GPIO_PDDR_PDD( LED_MASK ); //0x3C
	PTB->PDOR |= GPIO_PDOR_PDO( LED_MASK );
}

void init_buttons(void)
{
	// Set corresponding PTE pins (connected to LED's) for GPIO functionality
	// and enable interrupts
	PORTE->PCR[SW2] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW3] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW4] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);
	PORTE->PCR[SW5] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010);

	PORTE->PCR[SW6] = PORT_PCR_MUX(0x01);

	// Clear the Interrupt flags in Port E
	PORTE->ISFR = 0xFFFFFFFF;
}

void init_display(void)
{
	// Set display EN to output
	PORTE->PCR[EN] = ( PORT_PCR_MUX(0x01) );

	PTE->PDDR |= EN_MASK;
	// Set EN to 0 (MEANS ON!)
	PTE->PCOR = EN_MASK;


	// Display COL mux pins
	// A0 -> 8, A1 -> 10, A2 -> 6, A3 -> 11
	PORTA->PCR[A0] = ( PORT_PCR_MUX(0x01) ); // A0
	PORTA->PCR[A1] = ( PORT_PCR_MUX(0x01) ); // A1
	PORTA->PCR[A2] = ( PORT_PCR_MUX(0x01) ); // A2
	PORTA->PCR[A3] = ( PORT_PCR_MUX(0x01) ); // A3
	// Set COL mux pins to output
	PTA->PDDR = COL_MASK | ROW_MASK;


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
}

void init_pit(void)
{
	// Clear the Interrupt pending flag in the NVIC just in case
	NVIC->ICPR[2] = (1 << 27);

	// Enable the Interrupt in the NVIC
	NVIC->ISER[2] = (1 << 27);

	// Enable the clock on the PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;

	// Turn on the PIT
	PIT->MCR = 0x00;

	// Load Timer Value into Channel 0
	// LDVAL = (period/clock period) - 1
	// LDVAL = (0.0625 / (1 / 50,000,000)) - 1
	PIT->CHANNEL[0].LDVAL = 3124999; // period = 0.0625

	// Enable Interrupt and Start Timer
	PIT->CHANNEL[0].TCTRL = PIT_TCTRL_TIE_MASK | PIT_TCTRL_TEN_MASK;

	// Enable PIT0 Interrupt Channel in NVIC
	NVIC->ISER[2] |= (1 << 4);
}

void init(void)
{
	// Set the seed
	srand(time(NULL));

	// Turn on all port clocks
	SIM->SCGC5 = SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTE_MASK;

	init_leds();
	init_buttons();
	init_display();
	init_pit();
}


void irq_button_handler(void)
{
	// Forbid changing direction to the opposite

	if ( (PORTE->ISFR & (1 << SW2)) != 0 ) {
		PTB->PCOR = 1 << D9;
		snake_dir = snake_dir != DOWN ? UP : snake_dir;
	} else {
		PTB->PSOR = 1 << D9;
	}

	if ( (PORTE->ISFR & (1 << SW3)) != 0 ) {
		PTB->PCOR = 1 << D10;
		snake_dir = (snake_dir != LEFT && snake_dir != STOP) ? RIGHT : snake_dir;
	} else {
		PTB->PSOR = 1 << D10;
	}

	if ( (PORTE->ISFR & (1 << SW4)) != 0 ) {
		PTB->PCOR = 1 << D11;
		snake_dir = snake_dir != UP ? DOWN : snake_dir;
	} else {
		PTB->PSOR = 1 << D11;
	}

	if ( (PORTE->ISFR & (1 << SW5)) != 0 ) {
		PTB->PCOR = 1 << D12;
		snake_dir = snake_dir != RIGHT ? LEFT : snake_dir;
	} else {
		PTB->PSOR = 1 << D12;
	}
}


void set_cell_on(uint row, uint col)
{
	// Set all rows OFF
	PTA->PCOR = ROW_MASK;
	// Set the current row ON
	PTA->PSOR = 1 << ROW_ARR[row];


	// Set all cols OFF
	PTA->PCOR = COL_MASK;
	// Extract the individual bits from col_id and shift to the cols positions to create a mask
	PTA->PSOR = ((col	 ) & 1u) << A0
			  | ((col >> 1) & 1u) << A1
			  | ((col >> 2) & 1u) << A2
			  | ((col >> 3) & 1u) << A3;
}


void game_display_field(uint field[NUM_ROWS][NUM_COLS], uint dly)
{
	for (uint r = 0; r < NUM_ROWS; ++r) {
		for (uint c = 0; c < NUM_COLS; ++c) {
			if (field[r][c] < 1) {
				continue;
			}
			set_cell_on(r, c);
			delay(dly);
		}
	}
}

void game_restart(int lost)
{
	uint dly = 150;
	uint interval = 400;

	if (lost == 0) { // Display START message
		for (uint i = 0; i < interval; ++i) {
			game_display_field(text_start, dly);
		}
	} else { // Display LOST message
		for (uint i = 0; i < interval; ++i) {
			game_display_field(text_lost, dly);
		}
	}

	// Clear the field
	for (uint r = 0; r < NUM_ROWS; ++r) {
		for (uint c = 0; c < NUM_COLS; ++c) {
			field[r][c] = 0;
		}
	}

	// Reset global game variables
	snake_dir = STOP;
	snake_len = SNAKE_LEN_INIT;

	food_tick_counter = 0;

	int ini_x = 3;
	int ini_y = 9;

	// Set snake to its initial position
	for (uint i = 0; i < snake_len; ++i) {
		snake_body[i].x = ini_x;
		snake_body[i].y = (ini_y + i) % NUM_COLS;
		// Display the snake
		field[snake_body[i].x][snake_body[i].y] = 1;
	}
}

int  game_snake_collide(pos_t head_pos)
{
	// If snake collides with itself
	if (field[head_pos.x][head_pos.y] == 1) {
		game_restart(1); // Player LOST
		return 1;
	}

	// If snake collides with food
	if (field[head_pos.x][head_pos.y] == 2) {
		if (snake_len < SNAKE_LEN_MAX) {
			pos_t tail 	   = snake_body[snake_len-1];
			pos_t pre_tail = snake_body[snake_len-2];

			// Extend the tail in the direction it is currently facing
			snake_body[snake_len].x = tail.x + (tail.x - pre_tail.x);
			snake_body[snake_len].y = tail.y + (tail.y - pre_tail.y);

			++snake_len;
		}

		// Clear the food
		field[head_pos.x][head_pos.y] = 0;
	}
	return 0;
}

void game_snake_update(void)
{
	pos_t new_head = { snake_body[0].x, snake_body[0].y };

	new_head.x += (snake_dir == UP  ) ? -1 : (snake_dir == DOWN );
	new_head.y += (snake_dir == LEFT) ? -1 : (snake_dir == RIGHT);

	new_head.x %= 8;
	new_head.y %= 16;

	// Check collision
	if (game_snake_collide(new_head) != 0) {
		return;
	}

	// Clear the tail cell
	field[snake_body[snake_len-1].x][snake_body[snake_len-1].y] = 0;

	// Move the snake by one cell
	for (int i = snake_len-1; i > 0; --i) {
		snake_body[i] = snake_body[i-1];
		snake_body[i].x %= 8;
		snake_body[i].y %= 16;
	}

	// Set the new head
	snake_body[0] = new_head;
	field[snake_body[0].x][snake_body[0].y] = 1;
}

void game_spawn_food(void)
{
	int row, col;

	do {
		row = rand() % NUM_ROWS;
		col = rand() % NUM_COLS;
	} while (field[row][col] != 0); // Regenerate if the cell is already occupied

	field[row][col] = 2; // 2 means food
}

void game_update(void)
{
	if (snake_dir == STOP) {
		return;
	}

	game_snake_update();

	if (food_tick_counter == FOOD_TICKS) {
		game_spawn_food();
		food_tick_counter = 0;
	}

	++food_tick_counter;
}


void PORTE_IRQHandler()
{
	irq_button_handler();
	// Clear all Port E interrupt flags
	PORTE->ISFR = PORT_ISFR_ISF_MASK;
}

void PIT0_IRQHandler()
{
	// Toggle the LED
	PTB->PTOR = (1 << 5);

	game_update();

	// Clear the Interrupt Flag in the PIT module
	PIT->CHANNEL[0].TFLG = 0x01;
}



int main(void) {
	init();

	game_restart(0);

	for (;;) {
		game_display_field(field, GAME_DISPLAY_DELAY);
    }
    /* Never leave main */
    return 0;
}
