/*
 * Paddle.c
 *
 * Created: 02/10/2018 2:59:56 PM
 *  Author: DELL PC
 */
#include <stdint.h>
#include "lcd.h" 
#include "Paddle.h"


uint8_t prevL = 1;
uint8_t prevR = 126;

void generatePadLeft(uint8_t y)
{
	drawline(buff,1,prevL,1,(prevL+8), 0);
	drawline(buff,2,prevL,2,(prevL+8), 0);
	
	write_buffer(buff);
	if (y+8 > 63){
		y -=(64-y+8);
	}
	prevL=y;
	drawline(buff, 1, y, 1, (y+8), 1);
	drawline(buff, 2, y, 2, (y+8), 1);
	write_buffer(buff);
}

void generatePadRight(uint8_t y){

	drawline(buff,126,prevR,126,(prevR+8), 0);
	drawline(buff,125,prevR,125,(prevR+8), 0);
	write_buffer(buff);
	if (y+8 > 63){
		y -=(64-y+8);
		
	}
	prevR=y;
	drawline(buff, 126, y, 126, (y+8), 1);
	drawline(buff, 125, y, 125, (y+8), 1);
	write_buffer(buff);
}
