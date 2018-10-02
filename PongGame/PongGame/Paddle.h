/*
 * Paddle.h
 *
 * Created: 02/10/2018 3:00:16 PM
 *  Author: DELL PC
 */ 


#ifndef PADDLE_H_
#define PADDLE_H_


struct Paddle{
	uint8_t posx;
	uint8_t posy;
};

void generatePadLeft(uint8_t y);
void generatePadRight(uint8_t y);



#endif /* PADDLE_H_ */