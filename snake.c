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
void adc_init(void);
uint16_t adc_read(uint8_t ch);
void create_walls(void);
void process_loop(void);
void display_welcome_screen(void);
void draw_topbar(void);
void restart_round(void);
void create_food(void);
void food_eaten_test(void);
void snake_speed(long delay_time);
void create_snake_train(void);
void grow_snake_train(void);
void create_food_co_ords(int *x, int *y);
int tail_collision_test(int x, int y);
int move_snake(void);

int lives = 5;
int score = 0;
int snake_length = 2;
int start_x = 39, start_y = 21;

int w1_y1_y2, w1_x2;
int w2_x1_x2, w2_y2;
int w3_x1, w3_y1_y2, w3_x2;

volatile unsigned char btn_hists[NUM_BUTTONS];
volatile unsigned char btn_states[NUM_BUTTONS];
volatile int move_x = 0, move_y = 0;
volatile int walls_active = FALSE;
volatile long adc_value = 40;


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
uint16_t adc_result1;

void setup(){
	set_clock_speed(CPU_8MHz);

	// Turn on back light
	DDRC  |= 1 << PIN7;
  	PORTC |= 1 << PIN7;

  	DDRB &= ~(1<<PIN7 | 1<<PIN1);
  	DDRF &= ~(1<<PIN5 | 1<<PIN6);
  	DDRD &= ~(1<<PIN0 | 1<<PIN1);

	adc_init();

	srand((long)adc_read(4));

	// Initialise the LCD screen
	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();

	display_welcome_screen();
	create_walls();
	create_snake_train();
	create_food();

    TCCR0B &= ~(1 << WGM02);

    TCCR0B |= 1 << CS02;
    TCCR0B &= ~(1 << CS00);
    TCCR0B &= ~(1 << CS01);

    TIMSK0 |= 1 << TOIE0;

    TCCR1B &= ~(1 << WGM12);

    TCCR1B &= ~(1 << CS12);
    TCCR1B &= ~(1 << CS11);
    TCCR1B |= (1 << CS10);

    TIMSK1 |= 1 << TOIE1;

    // Globally enable interrupts
    sei();
}

void process_loop(){
	int round_over = FALSE;

	clear_screen();
	
	draw_topbar();
	draw_sprite(&chow_time);
	round_over = move_snake();
	food_eaten_test();

	if (walls_active){
		draw_line(0, w1_y1_y2, w1_x2, w1_y1_y2);
		draw_line(w2_x1_x2, 0, w2_x1_x2, w2_y2);
		draw_line(w3_x1, w3_y1_y2, w3_x2, w3_y1_y2);
	}

	for (int i = 0; i < snake_length; i++) {
		draw_sprite(&snake_train[i]);
	}


	show_screen();
	

	if (round_over){
		restart_round();
	}
	

	//_delay_ms((long)adc_read(1));

	snake_speed(adc_value);

}


void snake_speed(long delay_time) {
	while(delay_time--) {
		_delay_ms(1);
	}
}


void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);
 
    // ADC Enable and pre-scaler of 128
    // 8000000/128 = 62500
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
 
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}

void create_walls(){
    w1_y1_y2 = (rand() % 32) + 8;
    w1_x2 = (rand() % 32) + 10;

    w2_x1_x2 = (rand() % 53) + 30;
    w2_y2 = (rand() % 11) + 10;

    w3_x1 = 83;
    w3_y1_y2 = rand() % 40;
    w3_x2 = 73 - (rand() % 32);
}

void create_food_co_ords(int *x, int *y){
	int check_food_collision = FALSE;

	do{
		*x = rand() % 82;
		*y = rand() % 40;

		check_food_collision = FALSE;

		if (*x <= 30 && *y <= 8) check_food_collision = TRUE; // Re-roll if on topbar
		if (*x <= w1_x2 && *y <= w1_y1_y2 && *y >= w1_y1_y2-2) check_food_collision = TRUE; // Wall 1 test
		if (*x <= w2_x1_x2 && *x >= w2_x1_x2-2 && *y < w2_y2) check_food_collision = TRUE; // Wall 2 test
		if (*x <= w3_x1 && *x >= w3_x2 && *y <= w3_y1_y2 && *y >= w3_y1_y2-2) check_food_collision = TRUE; // Wall 3 test
		
	} while(check_food_collision);
}

void create_food(){
	int x = 0, y = 0;

	create_food_co_ords(&x, &y);
	init_sprite(&chow_time, x, y, 3, 3, food_bitmap);
}

void food_eaten_test(){
	int sx = snake_train[0].x, sy = snake_train[0].y;
	int fx = chow_time.x, fy = chow_time.y;

	if (((fx <= sx+2 && fx >= sx) || (sx <= fx+2 && sx >= fx)) && 
		((fy <= sy+2 && fy >= sy) || (sy <= fy+2 && sy >= fy))) {

		score++;
		if (snake_length <= 80) grow_snake_train(); // Prevent seg fault

		int x = chow_time.x;
		int y = chow_time.y;

		create_food_co_ords(&x, &y);

		chow_time.x = x;
		chow_time.y = y;
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
	char f_score[10];
	sprintf(f_score, "%03d (%d)",score , lives);
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

	if (btn_states[BTN_RIGHT] == BTN_STATE_DOWN){
		walls_active = TRUE;
	} else if (btn_states[BTN_LEFT] == BTN_STATE_DOWN){
		walls_active = FALSE;
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



ISR(TIMER1_OVF_vect) {
	float max_adc = 1023.0;
	adc_value = 50+((adc_read(1)*(long)100) / max_adc);
}