/*	CAB202: Assignment 2
*	Semester 2 2016
*
*	Ian Willam Daniel
*	n5372828
*/

#include <stdlib.h>
#include <math.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "cpu_speed.h"
#include "lcd.h"
#include "graphics.h"
#include "sprite.h"

/*#define AREA_X		(28)
#define AREA_Y		(24)*/

void setup(void);
void process_loop(void);
void display_welcome_screen(void);
void draw_topbar(void);
void move_snake(Sprite *snake);

int lives = 5;
int score = 0;
int move_x = 0;
int move_y = 0;

Sprite snake_train;


unsigned char snake_bitmap[] = {0xE0, 0xE0, 0xE0};
unsigned char food_bitmap[] = {0x40, 0xE0, 0x40};

int main(){
	setup();

	while(1){
		process_loop();
	}
	
	return 0;
}


void setup(){
	set_clock_speed(CPU_8MHz);

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
	//display_welcome_screen();

	/*for(int x = 0; x < AREA_X; x++){
		for(int y = 0; y < AREA_Y; y++){
			game_area[x][y] = 0;
		}
	}*/


	init_sprite(&snake_train, 0, 20, 3, 3, snake_bitmap);
}

void process_loop(){
	clear_screen();
	
	draw_topbar();
	move_snake(&snake_train);
	draw_sprite(&snake_train);
	show_screen();
	
	

	_delay_ms(10);


	show_screen();
}


void move_snake(Sprite *snake) {
    // Joystick - Move Up/Down
    if ((PIND >> 1) & 0b1 ){
    	move_x = 0; move_y= -1;
    } else if ((PINB >> 7) & 0b1 ){
    	move_x = 0; move_y = 1;
    }

    // Joystick - Move Left/Right
    if ((PINB >> 1) & 0b1 ){
    	move_x = -1; move_y = 0;
    } else if ((PIND >> 0) & 0b1 ){
    	move_x= 1; move_y = 0;
    }

	// Test for x axis wall collision
	if ((snake->x + move_x) == 83) {
		snake->x = -2;
	} else if ((snake->x + move_x) == -2) {
		snake->x = 83;
	} else {
		snake->x += move_x;
	}

	// Test for y axis wall collision
	if ((snake->y + move_y) == 46) {
		snake->y = -2;
	} else if ((snake->y + move_y) == -2) {
		snake->y = 46;
	} else {
		snake->y += move_y;
	}


}

void display_welcome_screen(){
	draw_string(42-25,15, "Ian Daniel");
	draw_string(42-20,25, "n5372828");
	show_screen();
	_delay_ms(500);
	///////////////////////////////////////////////  TODO - Change to 2 seconds
}

void draw_topbar(){
	char f_score[5];
	sprintf(f_score, "%.2d (%d)", score, lives);
	draw_string(0,0, f_score);
	show_screen();
}

