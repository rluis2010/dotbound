#include <raylib.h>
#include "input_map.h"


void update_input_map( InputMap* input_map )
{
	if( IsKeyDown( KEY_LEFT ) || IsKeyDown( KEY_A ) )
	{
		input_map->btn_left = 1;
	}
	else
	{
		input_map->btn_left = 0;
	}
	if( IsKeyDown( KEY_RIGHT ) || IsKeyDown( KEY_D ) )
	{
		input_map->btn_right = 1;
	}
	else
	{
		input_map->btn_right = 0;
	}
	if( IsKeyPressed( KEY_SPACE ) )
	{
		input_map->btn_jump = 1;
	}
	else
	{
		input_map->btn_jump = 0;
	}
}