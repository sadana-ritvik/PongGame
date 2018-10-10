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

// To check if two ADC values are same, only then plot the value!!
uint8_t touchTwoTimes = 1;
uint16_t x_old;
uint16_t y_old;

uint8_t screeenRefresh = 0;

// Coordinates from the ADC.
uint16_t x_coor;
uint16_t y_coor;

// For ball movements.
int8_t dx;
int8_t dy;

// initial position of the ball
uint8_t ballx = 63;
uint8_t bally = 28;
uint8_t future_x;
uint8_t future_y;

// For calculating actual pixel based on touch screen values.
uint8_t calc_x_pixel;
uint8_t calc_y_pixel;

// Properties of two paddles
struct Paddle pL;
struct Paddle pR;

// Score keeping.
uint8_t leftScore = 0;
uint8_t rightScore = 0;

// Variables to decide game modes.
uint8_t botMode = 0;
uint8_t acclMode = 0;
uint8_t touchMode = 0;
uint8_t gameModeSelected = 0;

// Variables for Accelerometer
uint16_t accReading =0;
const uint8_t size = 10;
uint16_t movingarray[10];
uint8_t pos =0;
uint16_t sum=0;
uint16_t movingAvr = 0;

// Timer setting for turning buzzer on/off.
uint16_t overflow = 0;

void GenerateBuzzerSound();

void generateGrid(){

    drawrect(buff, 0, 0, 127, 63, BLACK);
    for (int i= 0; i< 63 ; i+=8){
        drawline(buff, 63 , i, 63,i+4, BLACK);
    }
    write_buffer(buff);
}

void scoreBoard(int left, int right){
    
    drawchar(buff, 57,0, displayChar+left);
    drawchar(buff, 65,0, displayChar+right);
    write_buffer(buff);

}

void refreshScreen()
{
    cli();
    generateGrid();
    generatePadLeft(pL);
    generatePadRight(pR);
    scoreBoard(leftScore,rightScore);
    sei();
}

void resetScreen()
{
    cli();
    clear_buffer(buff);
    
    pL.posx = 2;
    pL.posy = 28;
    
    pR.posx = 2;
    pR.posy = 28;

    ballx = 63;
    bally = 28;
    
    uint8_t direction = rand()%5;

    dx = rand()%4+2;
    dy = rand()%4+2;
    
    if(direction < 2)
    {
        dx = -dx;
        dy = -dy;
    }

    generateGrid();
    generatePadLeft(pL);
    generatePadRight(pR);
    //drawcircle(buff, ballx, bally, 3, BLACK);
    scoreBoard(leftScore,rightScore);
    sei();
}
static inline void changeLCDtoRed()
{
    PORTB |= 0x05;
    _delay_ms(1000);
    PORTB &= ~0x05;
}

void updateBall()
{
	fillcircle(buff, ballx, bally, 3, 0);
    if(dy < 0)
    {
        future_y = bally+dy+2;
    }
    else
    {
        future_y = bally+dy-2;
    }
	if(bally+dy+2 >= 62 || bally+dy-2 <= 0)
	{
        GenerateBuzzerSound();
		dy = (-1)*dy;
	}
    if(botMode){
        if(ballx+dx+2 >= 124)
        { 
            GenerateBuzzerSound();
            pR.posy = bally+dy+2;
            generatePadRight(pR);
            dx = (-1)*dx;   
            
        }
        else if(ballx+dx-2 <= 3 && (future_y > pL.posy+4 || future_y < pL.posy-4))
        {
            rightScore++;
            GenerateBuzzerSound();
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx-2 <= 3 && (future_y <= pL.posy+4 && future_y >= pL.posy-4))
        {
            GenerateBuzzerSound();
            dx = (-1)*dx;
        }
        else
        {
           // Do nothing.
        }
    }
    else
    {
        if(ballx+dx+2 >= 124 && (future_y > pR.posy+4 || future_y < pR.posy-4))
        {
            leftScore++;
            GenerateBuzzerSound();
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx-2 <= 3 && (future_y > pL.posy+4 || future_y < pL.posy-4))
        {
            rightScore++;
            GenerateBuzzerSound();
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx+2 >= 124 && (future_y <= pR.posy+4 && future_y >= pR.posy-4))
        {
            GenerateBuzzerSound();
            dx = (-1)*dx;
        }
        else if(ballx+dx-2 <= 3 && (future_y <= pL.posy+4 && future_y >= pL.posy-4))
        {
            GenerateBuzzerSound();
            dx = (-1)*dx;
        }
        else
        {
            //nothing
        }
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
	
	PORTC &= ~(1 << PORTC0);
}

void getADCval() {

	getXval();
	getYval();
}

//Accelerometer Code
uint16_t movingAverage(uint16_t *ptrtoNums, uint16_t *ptrSum, uint16_t pos, uint16_t len, uint16_t NextNum){

    *ptrSum = *ptrSum - ptrtoNums[pos] + NextNum;

    ptrtoNums[pos] = NextNum;

    return (*ptrSum) / len;
}

uint16_t accelerometerReading()
{
    ADMUX &= 0xF0;
    ADMUX |= (1 << MUX2);
    _delay_ms(10);

    ADCSRA |= (1 << ADSC);
    while(!(ADCSRA & (1<<ADIF)));
    ADCSRA|=(1<<ADIF);

    movingAvr = movingAverage(movingarray, &sum, pos, size, ADC);
    pos++;
    if(pos >= size){
        pos = 0;
    }


    return movingAvr;
}


void displayGameOver()
{
    clear_buffer(buff);
    unsigned char word[] ="GAME OVER!!!";
    drawstring(buff, 35, 4,word);
    write_buffer(buff);

}

void displayGameMenu()
{
    clear_buffer(buff);
    unsigned char welcome[] = " WELCOME TO PONG ";
    unsigned char word1[] = "1.Player Vs Player";
    unsigned char word2[] = "2.Player Vs Computer";
    unsigned char word3[] = "3.Acclmtr Vs Computer";
    drawstring(buff, 4 , 0, welcome);
    drawstring(buff, 0, 2, word1);
    drawstring(buff, 0, 4, word2);
    drawstring(buff, 0, 6, word3);
    write_buffer(buff);
}

void GenerateBuzzerSound(){
    DDRB |= (1<<PORTB1);
    OCR1A = 10; //Frequency for 10 khz
    TCCR1B |= (1<<CS10); //Start the buzzer
    
    TCCR0B |= (1 << CS02)|(1 << CS00); //Start the timer0 for getting 0.5sec buzzer.

    while(overflow <3){
        _delay_ms(0.00000001);
    };
    overflow = 0;
    TCCR0B &= ~((1 << CS02)|(1 << CS00)); //Start the timer0 for getting 0.5sec buzzer.
    TCCR1B &= ~(1<<CS10);
    DDRB &= ~(1 << PORTB1);
    
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

    TIMSK0 |= (1 << TOIE0);
    
    // initialize counter
    TCNT0 = 0;
    // Timer1 for buzzer.
    TCCR1A |= (1<<COM1A0) | (1<<WGM12); //CTC and toggle on compare 
    sei();

	//lcd initialisation
	lcd_init();
	lcd_command(CMD_DISPLAY_ON);
	lcd_set_brightness(0x18);
	write_buffer(buff);
	_delay_ms(1000);
	clear_buffer(buff);
	
	uart_init();
	
	setupADC();
  
    while(1)
    {
        displayGameMenu();
        while(!gameModeSelected)
        {
            getADCval();
            touchTwoTimes++;
            if(touchTwoTimes%2 == 0)
            {
                touchTwoTimes = 0;
                if( (x_old == x_coor) && (y_old == y_coor) )
                {
                    calc_x_pixel = A*x_coor + B*y_coor + C;
                    calc_y_pixel = D*x_coor + E*y_coor + F;

                    if(calc_y_pixel >= 16 && calc_y_pixel <= 23)
                    {
                        botMode = 0;
                        touchMode = 1;
                        acclMode = 0;
                        gameModeSelected = 1;
                    }
                    else if (calc_y_pixel >= 32 && calc_y_pixel <=39)
                    {
                        botMode = 1;
                        touchMode = 1;
                        acclMode = 0;
                        gameModeSelected = 1;
                    }
                    else if (calc_y_pixel >= 48 && calc_y_pixel <=57)
                    {
                        botMode = 1;
                        touchMode = 0;
                        acclMode = 1;
                        gameModeSelected = 1;
                    }
                    else
                    { 
                        // do nothing 
                    }
                }
            }
            else
            {
                x_old = x_coor;
                y_old = y_coor;
            }
        }

    	resetScreen();

	    while (!(rightScore >= 3 || leftScore>=3))
	    {

		    updateBall();
		    if(touchMode)
		    {
                getADCval();
		
		        touchTwoTimes++;
		        if(touchTwoTimes%2 == 0)
		        {
			
			        touchTwoTimes = 0;

			        if( (x_old == x_coor) || (y_old == y_coor) )
			        {
				        calc_x_pixel = A*x_coor + B*y_coor + C;
				        calc_y_pixel = D*x_coor + E*y_coor + F;
				
				        if(calc_y_pixel <= 0)
					        calc_y_pixel = 4;
				
				        if(calc_x_pixel < 63)
				        {
					        pL.posy = calc_y_pixel;
					        generatePadLeft(pL);
				        }
				        if(!botMode && calc_x_pixel > 63 && calc_x_pixel < 127)
				        {
					        pR.posy = calc_y_pixel;
					        generatePadRight(pR);
				        }
			        }
		        }
		        else
		        {
			        x_old = x_coor;
			        y_old = y_coor;
		        }
	        }
            if(acclMode)
            {
                accReading = accelerometerReading();
               // printf("%d \n", accReading);
                if(accReading <= 303)
                {
                    accReading = 303;
                }
                else if(accReading >= 303 && accReading <= 330)
                {
                    // it's okay!!
                }
                else
                {
                    accReading = 336;
                }
                calc_y_pixel = ((accReading - 303)*56)/33+4;
                pL.posy = calc_y_pixel;
                generatePadLeft(pL);
               // printf("acc Reading: %u\n", accReading);                
            }
            _delay_ms(100);
           
            refreshScreen();
        }
        leftScore = 0;
        rightScore = 0;
        gameModeSelected = 0;

        displayGameOver();
        PORTB |= 0x01;
        _delay_ms(4000);
        PORTB &= ~0x01;

        resetScreen();
    }
}
ISR(TIMER0_OVF_vect)
{
    overflow++;
}
