# Pong Game #

## Brief Description about the project ##

This README documents whatever steps are necessary to get your application up and running. It also describes the code in brief details

### Importing the project(Windows + Atmel Studio). ###

* Clone the repo git clone [Pong Game](git@bitbucket.org:ritvik_sadana/ese519_pong.git)
* Go to ~/PongGame
* Import PongGame.atsln

## Game play ##

We have basically 3 elements in the game: Paddle, Ball and Screen/Grid.

### Paddle ###

Defined by a structure: 

```cpp
struct Paddle{
	uint8_t posx;
	uint8_t posy;
};
```

There are two paddles defined globally in main.c
```cpp
// Properties of two paddles
struct Paddle pL;
struct Paddle pR;
```
It gets input from either the touch screen or accelerometer which set it's "posy" during the game play.

```cpp
pR.posy = calc_y_pixel;
pL.posy = calc_y_pixel;
```

The movement of each paddle is controlled by its function:
```cpp
generatePadLeft(pL);
generatePadRight(pR);
```

### Ball ###

Since there is only one ball, we identify the ball using its x and y axis:
```cpp
// initial position of the ball
uint8_t ballx = 63;
uint8_t bally = 28;
```

The speed of ball is controlled by the dx and dy variables which also control is dynamics(up/down/left/right) when it hits the walls or paddles.
```cpp
// For ball movements.
int8_t dx;
int8_t dy;
```

### Screen ###

The screen further has two elements:
 
* Grid : The boundary walls and middle dashed lined
* Score: Scores of two players


