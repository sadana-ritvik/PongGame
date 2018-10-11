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
uint8_t paddleFollow = 0;

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

void GenerateBuzzerSound(uint16_t time);

void generateGrid(){

    drawrect(buff, 0, 0, 127, 63, BLACK);
    for (int i= 0; i< 63 ; i+=8){
        drawline(buff, 63 , i, 63,i+4, BLACK);
    }
    write_buffer(buff);
}

/* 
 * Function to update scoreboard with new score.
 */ 
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
    scoreBoard(leftScore,rightScore);
    sei();
}
static inline void changeLCDtoRed()
{
    PORTB |= 0x05;
    _delay_ms(1000);
    PORTB &= ~0x05;
}

/*
 * Function to update ball position and handle collisions with different elements.
 * There are different modes Bot vs Player, and Player Vs Player.
 *
 * If it hits vertical walls, the y-direction is reversed.
 * 
 * Bot vs Player:
 * If it is near the bot, the bot follows the ball. Once the balls hits the paddle,
 * it reverses its x-direction.
 * The ball is near to the player end, it checks if it hits the paddle or not.
 * If yes(hits paddle), then reverse x-direction. If no(hits wall), then increase score
 * and restart the game.
 *
 * Player Vs Player:
 * The ball is near to the player end, it checks if it hits the paddle or not.
 * If yes(hits paddle), then reverse x-direction. If no(hits wall), then increase score
 * and restart the game.
 * 
 */
void updateBall()
{
	fillcircle(buff, ballx, bally, 3, 0);
	
	// Make the paddle follow ball after certain x coordinate.
	if(botMode && ballx > 83 && dx > 0)
    {
        paddleFollow = 1;
	}
    if(paddleFollow){
        pR.posy = bally;
        generatePadRight(pR);
    }
	
	// Calculate future coordinates of ball to detect collision.
    if(dy < 0)
    {
        future_y = bally+dy+2;
    }
    else
    {
        future_y = bally+dy-2;
    }
	
	// If ball collides with the upper or lower wall, it reverses its y-direction.
	if(bally+dy+2 >= 62 || bally+dy-2 <= 0)
	{
        GenerateBuzzerSound(10);
		dy = (-1)*dy;
	}
	
	// Handle collisions of balls with paddles and left-right walls.
    if(botMode){
        if(ballx+dx+2 >= 124)
        { 
            GenerateBuzzerSound(10);
            pR.posy = bally+dy+2;
            generatePadRight(pR);
            paddleFollow = 0;
            dx = (-1)*dx;   
            
        }
        else if(ballx+dx-2 <= 3 && (future_y > pL.posy+4 || future_y < pL.posy-4))
        {
            rightScore++;
            GenerateBuzzerSound(15);
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx-2 <= 3 && (future_y <= pL.posy+4 && future_y >= pL.posy-4))
        {
            GenerateBuzzerSound(10);
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
            GenerateBuzzerSound(15);
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx-2 <= 3 && (future_y > pL.posy+4 || future_y < pL.posy-4))
        {
            rightScore++;
            GenerateBuzzerSound(15);
            changeLCDtoRed();
            resetScreen();
        }
        else if(ballx+dx+2 >= 124 && (future_y <= pR.posy+4 && future_y >= pR.posy-4))
        {
            GenerateBuzzerSound(10);
            dx = (-1)*dx;
        }
        else if(ballx+dx-2 <= 3 && (future_y <= pL.posy+4 && future_y >= pL.posy-4))
        {
            GenerateBuzzerSound(10);
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

/*
 * Set up ADC for getting touch screen and accelerometer values.
 * -> Set Ref as voltage 5
 * -> Enable ADC.
 * -> Set prescaler as 128.
 */
void setupADC(void){

	ADMUX |= (1 << REFS0); 	// Setting ADC to use VCC as reference voltage

	ADCSRA |=  (1 << ADEN) | (1<< ADPS1) | (1 << ADPS2) | (1 << ADPS0); // Turn ADC on,
																		//  Use pre-scaler as 128
	DIDR0 |= 1 << ADC0D;	// Disable digital input buffer.
}

/*
 * Function to get x_coor from the ADC.
 * -> Set direction of X- , X+ to input.
 * -> Set X- to high.
 * -> Select A0(Y-) pin of ADC.
 * -> Start the conversion and wait for it to complete.
 * -> Clear the ADC flag.
 * -> Read ADC value into x_coor.
 */
void getXval(){
	
	// Set Y- to ADC input.
	// Set X- , X+ to digital pins.
	DDRC  = (1 << PORTC1) | (1 << PORTC3);
	
	// Set X- high and X+ low.
	PORTC |= (1 << PORTC3);
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

/*
 * Function to get y_coor from the ADC.
 * -> Set direction of Y- , Y+ to input.
 * -> Set Y- to high.
 * -> Select A3(X-) pin of ADC.
 * -> Start the conversion and wait for it to complete.
 * -> Clear the ADC flag.
 * -> Read ADC value into y_coor.
 */
void getYval(){

    // Now set configuration for the y_coor.
	
    // Set Y-, Y+ to digital pins.
	DDRC = (1 << PORTC0) | (1 << PORTC2) ;
	// Set Y- high, Y+ low.
	PORTC |= (1 << PORTC0);
	// Set X- to ADC input.
	ADMUX |= (1 << MUX0) | (1 << MUX1);
	_delay_ms(10);

	//Start Conversion
	ADCSRA |= (1 << ADSC); 
	
	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));
	ADCSRA|=(1<<ADIF);
	// Read the y_coor here.
	y_coor = ADC;
	
	PORTC &= ~(1 << PORTC0);
}

/*
 * Function to get both X and Y coordinates from touchscreen.
 */
void getADCval() {

	getXval();
	getYval();
}

/*
 * Function to calculate moving average for smooth movements.
 * It keeps on adding/replacing values from the sensor to an array.
 * ptrSum is a pointer to a location which keeps track for the Sum.
 * returns the average of the arrray.
 *
 * Help from source: https://gist.github.com/bmccormack/d12f4bf0c96423d03f82
 */
uint16_t movingAverage(uint16_t *ptrtoNums, uint16_t *ptrSum, uint16_t pos, uint16_t len, uint16_t NextNum){

    *ptrSum = *ptrSum - ptrtoNums[pos] + NextNum;

    ptrtoNums[pos] = NextNum;

    return (*ptrSum) / len;
}


/*
 * Function to read accelerometer readings from ADC.
 * -> Sets A4 pin to ADC mode.
 * -> Starts ADC conversion and waits for it to complete.
 * -> Gets moving average and returns it to main to set paddle position.
 */
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

/*
 * Function to display GAME OVER!!!.
 */
void displayGameOver()
{
    clear_buffer(buff);
    unsigned char word[] ="GAME OVER!!!";
    drawstring(buff, 35, 4,word);
    write_buffer(buff);

}

/*
 * Function to display game Menu.
 * Displays the three modes.
 * 1. Player Vs Player.
 * 2. Player Vs Computer.
 * 3. Accelerometer vs Computer.
 */
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


/* 
 * Function to generate buzzer sound for a short time.
 * -> Starts Timer 0 to count the duration for which buzzer will be on.
 * -> Starts Timer 1 in CTC mode to make buzzer work.
 * -> Waits for Timer 0 to overflow 3 times.
 * -> Once overflow is reached, disable the buzzer.
 */
void GenerateBuzzerSound(uint16_t time){
    DDRB |= (1<<PORTB1);
    OCR1A = time; //Frequency for 10 khz
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

	
	/* Initialization for Timer 0 and 1 to use Buzzer */
	
	// Enable interrupt on timer 0.
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
			
			// Get touch input for selecting game mode.
			// The touch will only work only if two consecutive values of 
			// x and y coordinate respectively are same.
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

		// Run the game till either of the score reaches 3. 
		// This can be set to any value.
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
