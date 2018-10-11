# Pong Game #

## Brief Description about the project ##

This README documents whatever steps are necessary to get your application up and running. It also describes the code in brief details

### Importing the project(Windows + Atmel Studio). ###

* Clone the repo git clone [Pong Game](git@bitbucket.org:ritvik_sadana/ese519_pong.git)
* Go to ~/PongGame
* Import PongGame.atsln

## Game Elements ##

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
 
* Grid : The boundary walls and middle dashed lines
* Score: Scores of two players

## Game Flow ##

This sections describes the flow of game as a whole. It is represented as a pseudo-code.
```cpp

//setup: ADC, timers, LCD, uart

while(1)
{
	// Input game mode from the user using touch screen;
	gameMode = input;
	
	
	
	while (score < 3)
	{
		// Update ball and process the collision conditions.
		updateBall();
		
		if(gameMode == accelerometer)
		{
			// Start taking ADC values from the accelerometer
			y = ADC;
			calculated_y = y * ScalingFactor;
			PaddleLoction = calculated_y;
		}
		if(gameMode == touch)
		{
			x_coor = ADC(Y-);
			y_coor = ADC(X-);

			// A,B,C; D,E,F are coefficients obtained from touchscreen calibration
			calculated_x = A*x_coor + B*y_coor + C;
			calculated_x = D*x_coor + E*y_coor + F;

			updatePaddleLeft();
			updatePaddleRight();
		}
		refreshScreen();
	}
	resetScores;
	reset gameMode;
	
	display GameOver();
	
	resetScreen();
}
```
 
## Game play functions ##

This section describes various functions used to make the game work!

1. ```void updateBall()```
	* Function to update ball position and handle collisions with different elements.
	* If it hits vertical walls, the y-direction is reversed.
	* There are different modes Bot vs Player, and Player Vs Player for x-direction.
	* Bot vs Player:
		* If it is near the bot, the bot follows the ball. Once the balls hits the paddle,
		* it reverses its x-direction.
		* The ball is near to the player end, it checks if it hits the paddle or not.
		* If yes(hits paddle), then reverse x-direction. If no(hits wall), then increase score and restart the game.
	* Player Vs Player:
		* The ball is near to the player end, it checks if it hits the paddle or not.
		* If yes(hits paddle), then reverse x-direction. If no(hits wall), then increase score and restart the game.
		
2. ```void setupADC()```
	* Set up ADC for getting touch screen and accelerometer values.
	* Set Ref as voltage 5
	* Enable ADC.
	* Set prescaler as 128.

3. ```void getXval()```
	* Function to get x_coor from the ADC.
	* Set direction of X- , X+ to input.
	* Set X- to high.
	* Select A0(Y-) pin of ADC.
	* Start the conversion and wait for it to complete.
	* Clear the ADC flag.
	* Read ADC value into x_coor.

4. ```getYval()```
	* Function to get y_coor from the ADC.
	* Set direction of Y- , Y+ to input.
	* Set Y- to high.
	* Select A3(X-) pin of ADC.
	* Start the conversion and wait for it to complete.
	* Clear the ADC flag.
	* Read ADC value into y_coor.

5. ```uint16_t accelerometerReading()```
	* Function to read accelerometer readings from ADC.
	* Sets A4 pin to ADC mode.
	* Starts ADC conversion and waits for it to complete.
	* Gets moving average and returns it to main to set paddle position.

6. ```uint16_t movingAverage(uint16_t *ptrtoNums, uint16_t *ptrSum, uint16_t pos, uint16_t len, uint16_t NextNum)```
	* Function to calculate moving average for smooth movements.
	* It keeps on adding/replacing values from the sensor to an array.
	* ptrSum is a pointer to a location which keeps track for the Sum.
	* returns the average of the arrray.
	* Help from source: https://gist.github.com/bmccormack/d12f4bf0c96423d03f82
	
7. ```void GenerateBuzzerSound(uint16_t time)```
	* Function to generate buzzer sound for a short time.
	* Starts Timer 0 to count the duration for which buzzer will be on.
	* Starts Timer 1 in CTC mode to make buzzer work.
	* Waits for Timer 0 to overflow 3 times.
	* Once overflow is reached, disable the buzzer.

8. ```static inline void changeLCDtoRed()```
	* Function to change LCD backlight color to Red for 1 second.

9. ```void refreshScreen()```
	* Function to refresh screen on a regular basis.
	* It disables the interrupts.
	* Generates paddles, grid and scoreboard.
	* Enables the interrupts again.

10. ```void resetScreen()```
	* Function to reset the screen to default values. 
	* Also selects random direction and speed for the ball.

11. ```void generatePadLeft(struct Paddle p) && void generatePadRight(struct Paddle p)```
	* Location: Paddle.c
	* Function to generate paddles using the y coordinates set in main function.
	




