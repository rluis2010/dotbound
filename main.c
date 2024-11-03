#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#include "defs.h"


//#include "tilemap.h"
//#include "tileset_img.h"
//#include "ldtk_tools.c"
#include "screen_startmenu.h"
#include "screen_gameplay.h"
#include "screen_end.h"





//----------------------------------------------------------------
// Global variables
//----------------------------------------------------------------


// select the current screen of the game
static GameScreen current_screen = SCREEN_TITLE;//SCREEN_GAMEPLAY;//SCREEN_ENDING;//


//----------------------------------------------------------------
// Function declarations
//----------------------------------------------------------------
static void update_draw_frame( void );



int main()
{
	// initialize window
	SetTraceLogLevel( 10 );
	
	SetConfigFlags( FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT );
    InitWindow( screen_width, screen_height, "raylib core example" );
	SetExitKey( KEY_NULL );

	InitAudioDevice();

	// Initialize screens
	startmenu_screen_initialize();
	gameplay_screen_initialize();
	end_screen_initialize();

	// Main loop
    SetTargetFPS( 60 );
    while( !WindowShouldClose() )
    {
		update_draw_frame();
    }
	// De-initialization
	startmenu_screen_terminate();
	gameplay_screen_terminate();
	end_screen_terminate();

	CloseAudioDevice();

    CloseWindow();

    return 0;
}

static void update_draw_frame( void )
{
	GameScreen new_screen = SCREEN_NONE;
	//printf( "%d, %f\n", GetFPS(), GetFrameTime() );
	switch( current_screen )
	{
		case SCREEN_NONE:
			break;
		case SCREEN_TITLE:
			new_screen = startmenu_screen_run();
			if( new_screen != SCREEN_NONE )
			{
				startmenu_screen_terminate();
				gameplay_screen_initialize();
				current_screen = new_screen;
			}
			break;
		case SCREEN_GAMEPLAY:
			new_screen = gameplay_screen_run();
			if( new_screen != SCREEN_NONE )
			{
				gameplay_screen_terminate();
				end_screen_initialize();
				current_screen = new_screen;
			}
			break;
		case SCREEN_ENDING:
			new_screen = end_screen_run();
			if( new_screen != SCREEN_NONE )
			{
				end_screen_terminate();
				startmenu_screen_initialize();
				current_screen = new_screen;
			}
			break;
	}
}




