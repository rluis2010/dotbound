#ifndef DEFS_H
#define DEFS_H

#define USE_EXTERNAL_RESOURCE_FILES
#define STARTING_LEVEL 8

// a few macros
#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))
#define CANDY_DARK ( Color ){ 21, 18, 18, 255 }

typedef enum
{
	SCREEN_NONE = 0,
	SCREEN_TITLE,
	SCREEN_GAMEPLAY,
	SCREEN_ENDING,
} GameScreen;




// size of the window
static const int screen_width = 1024;
static const int screen_height = 1024;

// size of the game screen
static const int gameScreenWidth = 256;
static const int gameScreenHeight = 256;




#endif