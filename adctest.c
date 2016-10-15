#include <stdlib.h>
#include <math.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "cpu_speed.h"
#include "lcd.h"
#include "graphics.h"
#include "sprite.h"


int score = 0;

int main() {
	set_clock_speed(CPU_8MHz);
	// Turn on back light
	DDRC  |= 1 << PIN7;
  	PORTC |= 1 << PIN7;

	lcd_init(LCD_DEFAULT_CONTRAST);
	clear_screen();



	DDRF = 0b11111111;





	while(1){
		clear_screen();


		char f_score[5];
		sprintf(f_score, "%d", score);
		draw_string(10,10, f_score);


		show_screen();
	}

	return 0;
}