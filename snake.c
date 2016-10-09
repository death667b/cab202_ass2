/*	CAB202: Assignment 2
*	Semester 2 2016
*
*	Ian Willam Daniel
*	n5372828
*/

#include <stdlib.h>
#include <math.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

#include "cpu_speed.h"
#include "lcd.h"
#include "graphics.h"
#include "sprite.h"

#define TRUE  (1)
#define FALSE (0)

#define NUM_BUTTONS 6
#define BTN_DPAD_LEFT 0
#define BTN_DPAD_RIGHT 1
#define BTN_DPAD_UP 2
#define BTN_DPAD_DOWN 3
#define BTN_LEFT 4
#define BTN_RIGHT 5

#define BTN_STATE_UP 0
#define BTN_STATE_DOWN 1

void setup(void);
void process_loop(void);
void display_welcome_screen(void);
void draw_topbar(void);
void restart_round(void);
void create_food(void);
void food_eaten_test(void);
void create_snake_train(void);
void grow_snake_train(void);
int tail_collision_test(int x, int y);
int move_snake(void);

int lives = 5;
int score = 0;
int snake_length = 2;
int start_x = 39, start_y = 21;

volatile unsigned char btn_hists[NUM_BUTTONS];
volatile unsigned char btn_states[NUM_BUTTONS];
volatile int move_x = 0, move_y = 0;


Sprite *snake_train = NULL;
Sprite chow_time;


unsigned char snake_bitmap[] = {0xE0, 0xE0, 0xE0};
unsigned char food_bitmap[] = {0x40, 0xE0, 0x40};

int main(){
	setup();

	while(TRUE){
		process_loop();
	}
	
	return 0;
}


void setup(){
	set_clock_speed(CPU_8MHz);
	srand(8);
	// Turn on back light
	DDRC  |= 1 << PIN7;
  	PORTC |= 1 << PIN7;

	// Get our data directions set up as we please (everything is an output unless specified...)
	DDRB = 0b01111100;
	DDRF = 0b10011111;
	DDRD = 0b11111100;

	// Initialise the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();

	display_welcome_screen();
	create_snake_train();
	create_food();

    TCCR0B &= ~(1 << WGM02);

    TCCR0B |= 1 << CS02;
    TCCR0B &= ~(1 << CS00);
    TCCR0B &= ~(1 << CS01);

    TIMSK0 |= 1 << TOIE0;

    // Globally enable interrupts
    // TODO
    sei();

}

void process_loop(){
	int round_over = FALSE;

	clear_screen();
	
	draw_topbar();
	draw_sprite(&chow_time);
	round_over = move_snake();
	food_eaten_test();
	//round_over = tail_collision_test();


	for (int i = 0; i < snake_length; i++) {
		draw_sprite(&snake_train[i]);
	}
	show_screen();
	

	if (round_over){
		restart_round();
	}
	

	_delay_ms(350);



}

void create_food(){
	int x = rand() % 82;
	int y = rand() % 40;

	while (x <= 30 && y <= 8){
		x = rand() % 82;
		y = rand() % 40;
	}

	init_sprite(&chow_time, x, y, 3, 3, food_bitmap);
}

void food_eaten_test(){
	int sx = snake_train[0].x, sy = snake_train[0].y;
	int fx = chow_time.x, fy = chow_time.y;

	if (((fx <= sx+2 && fx >= sx) || (sx <= fx+2 && sx >= fx)) && 
		((fy <= sy+2 && fy >= sy) || (sy <= fy+2 && sy >= fy))) {
		score++;
		grow_snake_train();

		chow_time.x = rand() % 82;
		chow_time.y = rand() % 40;

		while (chow_time.x <= 30 && chow_time.y <= 8){
			chow_time.x = rand() % 82;
			chow_time.y = rand() % 40;
		}
	}
}


int move_snake() {
	int newx = 0, newy = 0;
	int oldx = 0, oldy = 0;
	int round_over = FALSE;

	// Test for x axis wall collision
	if ((snake_train[0].x + move_x) >= 82) {
		newx = 0;
	} else if ((snake_train[0].x + move_x) <= -1) {
		newx = 81;
	} else {
		newx = snake_train[0].x + (move_x*3);
	}

	// Test for y axis wall collision
	if ((snake_train[0].y + move_y) >= 46) {
		newy = 0;
	} else if ((snake_train[0].y + move_y) <= -1) {
		newy = 45;
	} else {
		newy = snake_train[0].y + (move_y*3);
	}

	round_over = tail_collision_test(newx, newy);

	// Move Tail
	if ( move_y != 0 || move_x != 0 ){
		for (int i = 0; i < snake_length; i++) {
		    oldx = snake_train[i].x;
		    oldy = snake_train[i].y;
	
		    snake_train[i].x = newx;
		    snake_train[i].y = newy;
	
		    newx = oldx;
		    newy = oldy;
		}
	}

	return round_over;
}

void restart_round(){
	_delay_ms(1000);
	lives--;
	move_x = 0;
	move_y = 0;

	snake_length = 2;
	create_snake_train();
}

void grow_snake_train(){
	int x = snake_train[snake_length-1].x;
	int y = snake_train[snake_length-1].y;

	snake_length++;
	snake_train = realloc(snake_train, snake_length * sizeof(Sprite));
	
	init_sprite(&snake_train[snake_length-1], x, y, 3, 3, snake_bitmap);
}


void create_snake_train(){
	snake_train = realloc(snake_train, snake_length * sizeof(Sprite));
	
	for (int i = 0; i < snake_length; i++) {
		init_sprite(&snake_train[i], start_x-(i*3), start_y, 3, 3, snake_bitmap);
	}
}


int tail_collision_test(int x, int y){
	int round_over = FALSE;

	for (int i = 1; i < snake_length; i++) {
		if (move_x == 0 && (move_y == -1 || move_y == 1)){ // Moving Up or Down
			if ((snake_train[i].x == x+0 && snake_train[i].y == y+0) ||
					(snake_train[i].x == x+1 && snake_train[i].y == y+0) ||
					(snake_train[i].x == x+2 && snake_train[i].y == y+0)) {
				round_over = TRUE;
			}
		} else if ((move_x == -1 || move_x == 1) && move_y == 0){ // Moving Left or Right
			if ((snake_train[i].x == x+0 && snake_train[i].y == y+0) ||
					(snake_train[i].x == x+0 && snake_train[i].y == y+1) ||
					(snake_train[i].x == x+0 && snake_train[i].y == y+2)) {
				round_over = TRUE;
			}
		}
	}

	return round_over;
}

void display_welcome_screen(){
	draw_string(42-25,15, "Ian Daniel");
	draw_string(42-20,25, "n5372828");
	show_screen();
	_delay_ms(300);
}

void draw_topbar(){
	char f_score[5];
	sprintf(f_score, "%.2d (%d)", score, lives);
	draw_string(0,0, f_score);
	show_screen();
}

ISR(TIMER0_OVF_vect) {
    for (int i = 0; i < NUM_BUTTONS; i++){
        btn_hists[i]=btn_hists[i]<<1;
    }

    btn_hists[BTN_DPAD_LEFT]  |= ((PINB>>PIN1)&1)<<0;
    btn_hists[BTN_DPAD_RIGHT] |= ((PIND>>PIN0)&1)<<0;
    btn_hists[BTN_DPAD_UP]    |= ((PIND>>PIN1)&1)<<0;
    btn_hists[BTN_DPAD_DOWN]  |= ((PINB>>PIN7)&1)<<0;
    btn_hists[BTN_LEFT]       |= ((PINF>>PIN6)&1)<<0;
    btn_hists[BTN_RIGHT]      |= ((PINF>>PIN5)&1)<<0;

    for (int i = 0; i < NUM_BUTTONS; i++){
        if (btn_hists[i] == 0xFF && btn_states[i] == BTN_STATE_UP) {
            btn_states[i] = BTN_STATE_DOWN;
        } else if (btn_hists[i] == 0x0 && btn_states[i] == BTN_STATE_DOWN) {
            btn_states[i] = BTN_STATE_UP;
        }
    }

	// Joystick - Move Up/Down
	if (btn_states[BTN_DPAD_UP] == BTN_STATE_DOWN){
		move_x = 0; move_y= -1;
	} else if (btn_states[BTN_DPAD_DOWN] == BTN_STATE_DOWN){
		move_x = 0; move_y = 1;
	}

	// Joystick - Move Left/Right
	if (btn_states[BTN_DPAD_LEFT] == BTN_STATE_DOWN){
		move_x = -1; move_y = 0;
	} else if (btn_states[BTN_DPAD_RIGHT] == BTN_STATE_DOWN){
		move_x= 1; move_y = 0;
	}
}