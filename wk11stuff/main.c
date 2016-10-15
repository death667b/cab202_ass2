#include <avr/io.h>
#include <stdio.h>
#include <cpu_speed.h>
#include "graphics.h"
#include "sprite.h"
#include "lcd.h"

void setup(void);
void setup_adc(void);
int16_t read_adc(int8_t ch);

int main(){
    setup();    
    setup_adc();
    while(1){
        clear_screen();
        int16_t adc = read_adc(1);
        char res[10];
        sprintf(res,"%d",adc);
        draw_string(10,10,res);
        show_screen();
    }
    return 0;
}

void setup(){
    set_clock_speed(CPU_8MHz);
    DDRC  |= 1 << PIN7;
    PORTC |= 1 << PIN7;
    lcd_init(LCD_DEFAULT_CONTRAST);
    clear_screen();
    show_screen();
}

void setup_adc(){
    ADMUX  |= 1 << REFS0;
    ADCSRA |= 1 << ADEN | 1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0;

}

int16_t read_adc(int8_t ch){
    //0bxxxxxxxx & 00000111
    ADMUX &= 0b11111000;
    ADMUX |= (ch & ~(0xF8));
    ADCSRA |= 1 << ADSC;
    while (ADCSRA & (1 << ADSC));

    return ADC;
}
