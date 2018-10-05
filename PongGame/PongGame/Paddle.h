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

void generatePadLeft(struct Paddle p);
void generatePadRight(struct Paddle p);



#endif /* PADDLE_H_ */