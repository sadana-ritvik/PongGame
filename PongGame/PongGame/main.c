#define F_CPU 16000000L

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <util/delay.h>
#include "lcd.h"
#include "uart.h"

#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001

char displayChar = 0;

uint16_t x_coor;
uint16_t y_coor;

void setupADC(void){

	ADMUX |= (1 << REFS0); 	// Setting ADC to use VCC as reference voltage
	//  Use A0 pin to read input

	ADCSRA |= (1 << ADATE) | (1 << ADEN) | (1<< ADPS1) | (1 << ADPS2) | (1 << ADPS0); // Turn ADC on,
														 //  Use pre-scaler as 4
														 //  Set auto-trigger.
	DIDR0 |= 1 << ADC0D;
}

void adcStartConversion(void){
	ADCSRA |= (1 << ADSC);
}

void getADCval() {

	// Disable the ADC mode
	//ADMUX &= ~(1 << MUX3);
	
	// Set Y- Y+ to ADC input.
	//ADMUX |= 0xf0;
	// Set X- , X+ to digital pins.
	DDRC  = (1 << PORTC1) | (1 << PORTC3);
	// Set X- high and X+ low.
	PORTC |= (1 << PORTC3);
	PORTC &= ~(1 << PORTC1);
	
	// Read the x_coor here.
	x_coor = ADC;
	
	// Now set configuration for the y_coor.
	
	// Set Y-, Y+ to digital pins.
	DDRC = (1 << PORTC0) | (1 << PORTC2);
	// Set Y- high, Y+ low.
	PORTC |= (1 << PORTC0);
	PORTC &= ~(1 << PORTC2);
	
	// Set X- X+ to ADC input.
	ADMUX |= (1 << MUX0) | (1 << MUX1);
	
	// Read the y_coor here.
	y_coor = ADC;
	//PORTC &= ~(1<<PORTC0);
	
	
}

int main(void)
{
	//setting up the gpio for backlight
	DDRD |= 0x80;
	PORTD &= ~0x80;
	PORTD |= 0x00;
	
	DDRB |= 0x05;
	PORTB &= ~0x05;
	PORTB |= 0x00;
	
	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	
	uart_init();
	
	setupADC();
	adcStartConversion();
	
	while (1)
	{
// 		drawchar(buff,0,0,displayChar);
// 		write_buffer(buff);
// 		_delay_ms(5000);
// 		displayChar++;
// 		setpixel(buff,1,0, BLACK);
// 		setpixel(buff,1,1, BLACK);
// 		setpixel(buff,1,2, BLACK);
// 		setpixel(buff,1,3, BLACK);
// 		setpixel(buff,1,4, BLACK);
// 		
		//clearpixel(buff, 1,1);
		
// 		drawline(buff, 0, 0, 3, 0, BLACK);
// 		drawline(buff, 127, 63, 0, 0, BLACK);
// 		drawline(buff, 0, 0, 0, 3, BLACK);
		
// 		unsigned char word[] = "home" ;
// 		drawstring(buff, 0, 2, word);
		
//		drawrect(buff, 0,0, 80, 60, BLACK);
//		fillrect(buff, 0,0, 80, 60, BLACK);

		drawcircle(buff, 15, 15, 8, BLACK);

		write_buffer(buff);
		getADCval();
		
		//uint16_t f = ADC;
		printf("x: %u, y: %u \n", x_coor, y_coor);
	}
	
	
}

