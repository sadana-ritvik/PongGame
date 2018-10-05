/*
 * Paddle.c
 *
 * Created: 02/10/2018 2:59:56 PM
 *  Author: DELL PC
 */
#include <stdint.h>
#include "lcd.h" 
#include "Paddle.h"


uint8_t prevL = 4;
uint8_t prevR = 4;

void generatePadLeft(struct Paddle p)
{
	// Clear the paddle from last position.
	drawline(buff, 2, prevL-4, 2, (prevL+4), 0);
	drawline(buff, 3, prevL-4, 3, (prevL+4), 0);
	write_buffer(buff);
	
	// Limit the paddle boundary.
	if (p.posy+4 > 63){
		p.posy = 63;
	}
	if(p.posy-4 < 0){
		p.posy = 4;
	}
	prevL=p.posy;
	
	// Draw the new paddle.
	drawline(buff, 2, p.posy-4, 2, (p.posy+4), 1);
	drawline(buff, 3, p.posy-4, 3, (p.posy+4), 1);
	write_buffer(buff);
}

void generatePadRight(struct Paddle p){

	// Clear the paddle from last position.
	drawline(buff, 125, prevR-4, 125, (prevR+4), 0);
	drawline(buff, 124, prevR-4, 124, (prevR+4), 0);
	write_buffer(buff);
	
	// Limit the paddle boundary.
	if (p.posy+4 > 63){
		p.posy = 63;
	}
	if(p.posy-4 <= 0){
		p.posy = 4;
	}
	prevR=p.posy;
	
	// Draw the new paddle.
	drawline(buff, 125, p.posy-4, 125, (p.posy+4), 1);
	drawline(buff, 124, p.posy-4, 124, (p.posy+4), 1);
	write_buffer(buff);
}
