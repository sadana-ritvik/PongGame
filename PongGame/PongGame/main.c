#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "uart.h"
#include <util/delay.h>
#include "Paddle.h"
#include "lcd.h"

#define FREQ 16000000
#define BAUD 9600
#define HIGH 1
#define LOW 0
#define BUFFER 1024
#define BLACK 0x000001

// Touchscreen calibration constants.
const float A =	0.141787;
const float B = -0.00957;
const float C = -2.36838;

const float D =	-0.00268;
const float E = -0.11792;
const float F = 101.9308;


char displayChar = '0';

// To check if two ADC values are same, ony then plot the value!!
uint8_t touchTwoTimes = 1;
uint16_t x_old;
uint16_t y_old;

// Coordinates from the ADC.
uint16_t x_coor;
uint16_t y_coor;

// For ball movements.
int8_t dx;
int8_t dy;

uint8_t ballx = 63;
uint8_t bally = 28;


void updateBall()
{
	fillcircle(buff, ballx, bally, 3, 0);
	if( ballx+dx+2 >= 126 || ballx+dx-2 <= 0)
	{
		dx = (-1)*dx;
	}
	if(bally+dy+2 >= 63 || bally+dy-2 <= 0)
	{
		dy = (-1)*dy;
	}
	ballx += dx;
	bally += dy;
	fillcircle(buff, ballx, bally, 3, BLACK);
	write_buffer(buff);
}

void setupADC(void){

	ADMUX |= (1 << REFS0); 	// Setting ADC to use VCC as reference voltage
	//  Use A0 pin to read input

	ADCSRA |=  (1 << ADEN) | (1<< ADPS1) | (1 << ADPS2) | (1 << ADPS0); // Turn ADC on,
	//  Use pre-scaler as 4
	//  Set auto-trigger.
	DIDR0 |= 1 << ADC0D;
}

void adcStartConversion(void){
	ADCSRA |= (1 << ADSC);
}

void TouchStandby(){
	//Put To Standby
	DDRC  = 0x02; //Setting only C1 to output
	PORTC |= (1<<PORTC0);
}

void getXval(){
	// Disable the ADC mode
	//ADMUX &= ~(1 << MUX3);
	
	// Set Y- to ADC input.
	// Set X- , X+ to digital pins.

	DDRC  = (1 << PORTC1) | (1 << PORTC3);
	// Set X- high and X+ low.
	PORTC |= (1 << PORTC3);
	//PORTC &= ~(1 << PORTC1);  //0000 1101
	ADMUX &= 0xF0;
	_delay_ms(10);

	ADCSRA |= (1 << ADSC); //Start Conversion
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF);

	// Read the x_coor here.
	x_coor = ADC;
	//TouchStandby();
	PORTC &= ~(1 << PORTC3);
}

void getYval(){

	// Now set configuration for the y_coor.
	
	// Set Y-, Y+ to digital pins.
	DDRC = (1 << PORTC0) | (1 << PORTC2) ;
	// Set Y- high, Y+ low.
	PORTC |= (1 << PORTC0);
	// Set X- to ADC input.
	ADMUX |= (1 << MUX0) | (1 << MUX1);
	_delay_ms(10);

	ADCSRA |= (1 << ADSC); //Start Conversion
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF);
	// Read the y_coor here.
	y_coor = ADC;
	//TouchStandby();
	PORTC &= ~(1 << PORTC0);
}

void getADCval() {

	getXval();
	getYval();
}


void generateGrid(){

	drawrect(buff, 0, 0, 127, 63, BLACK);
	for (int i= 0; i< 63 ; i+=8){
		drawline(buff, 63 , i, 63,i+4, BLACK);
	}
	write_buffer(buff);
}

void scoreBoard(int left, int right){
	
	drawchar(buff, 57,0, displayChar);
	drawchar(buff, 65,0, displayChar);
	write_buffer(buff);

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
	
	//For Calibration
// 	setpixel(buff,12,6, BLACK);
// 	setpixel(buff,114,31, BLACK);
// 	setpixel(buff,63,57, BLACK);
// 	write_buffer(buff);


	generateGrid();
	generatePadLeft(28);
	generatePadRight(28);
	drawcircle(buff, 63, 28, 3, BLACK);
	
	scoreBoard(0,0);

	uint8_t calc_x_pixel;
	uint8_t calc_y_pixel;
	
	int direction = rand()%5;

	dx = rand()%2+1;
	dy = rand()%2+1;
	
	if(direction < 2)
		dy = -dy;
	

	while (1)
	{
		//Blinking circle
// 		 fillcircle(buff,12,12,2,BLACK);
// 		 _delay_ms(100);
// 		 fillcircle(buff,12,12,2,0);
// 		 _delay_ms(100);

		updateBall();
		_delay_ms(10);
		getADCval();
		
		touchTwoTimes++;
		if(touchTwoTimes%2 == 0)
		{
			
			touchTwoTimes = 0;
			//printf("x: %u y: %u\n", x_old, y_old );
			if( (x_old == x_coor) && (y_old == y_coor) )
			{
				calc_x_pixel = A*x_coor + B*y_coor + C;
				calc_y_pixel = D*x_coor + E*y_coor + F;
				
				//printf("x_calc: %u y_calc: %u\n", calc_x_pixel, calc_y_pixel);
				
				//setpixel(buff, calc_x_pixel, calc_y_pixel, BLACK);
				//write_buffer(buff);
				if(calc_x_pixel < 63)
					generatePadLeft(calc_y_pixel);
				else
					generatePadRight(calc_y_pixel);
			}
		}
		else
		{
			x_old = x_coor;
			y_old = y_coor;
		}
		//_delay_ms(10);
	}
	
	
}


