#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include "defs.h"
#include "ldtk_tools.h"
#include "input_map.h"
#include "player_controller.h"
#include "aseprite_tools.h"

#include "tilemap.h"
#include "tileset_img.h"
#include "player_img.h"
#include "player_anim.h"
#include "dots_img.h"
#include "dots_anim.h"
#include "background_img.h"


//--------------------------------------------------------------------
// Type definitions
//--------------------------------------------------------------------
// structure for the dots
typedef enum
{
	DOT_INACTIVE = 0,
	DOT_IDLE,
	DOT_ACTIVE
} DotState;
typedef enum
{
	DOT_NOT_ACQUIRED = 0,
	DOT_ACQUIRED_OK,
	DOT_ACQUIRED_NOK,
	DOT_ACQUIRED_FINAL,
} DotAquired;
typedef struct
{
	const LDtkEntity_dot* d;
	ASEpriteAnim* a;
	DotState ds;
	
} Dot;

// structure for the game state
typedef struct
{
	int current_level_idx;
	const LDtkLevel* current_level;
	const LDtkLayer* collision_layer;
	const LDtkLayer* hazards_layer;
	PlayerController p;
	PlayerMotionState pm;
	int ndots;
	Dot* dots;
	int dots_timer;
	int finished_level;
	int nxt_level_timer;
	int skip_timer;
} GamePlayState;



//--------------------------------------------------------------------
// Variables
//--------------------------------------------------------------------
// game state
static GamePlayState state = { 0 };
// texture to draw the screen before scaling
static RenderTexture2D target;
// some colors
static Color dark;
// textures
static Texture2D tileset;
static Texture2D player_texture;
static Texture2D dots_texture;
static Texture2D background_texture;
// player input
static InputMap input;

static float background_pos = 0.0f;

//--------------------------------------------------------------------
// Function definitions
//--------------------------------------------------------------------
static void initialize_gameplay_screen( void );
static void initialize_level( const LDtkLevel* level );
static void update_gameplay_screen( void );
static void draw_player( GamePlayState* s, PlayerMotionState pm );
static void draw_dots( GamePlayState* s );
static void draw_lines( GamePlayState* s );
static DotAquired update_dots( GamePlayState* s );




static void initialize_gameplay_screen( void )
{
	dark = GetColor( 0x151212ff );

	// initialize render texture (to scale to window size )
	target = LoadRenderTexture( gameScreenWidth, gameScreenHeight);
	SetTextureFilter( target.texture, TEXTURE_FILTER_POINT );


	// initialize texture for the tileset
	Image tileset_image = { 0 };
	tileset_image.format = 7;
    tileset_image.width = img_data_tileset.size_x;
    tileset_image.height = img_data_tileset.size_y;
    tileset_image.data = img_data_tileset.data;
    tileset_image.mipmaps = 1;
    tileset = LoadTextureFromImage( tileset_image );

	// initialize texture for the player
	Image player_image = { 0 };
	player_image.format = 7;
    player_image.width = img_data_player.size_x;
    player_image.height = img_data_player.size_y;
    player_image.data = img_data_player.data;
    player_image.mipmaps = 1;
    player_texture = LoadTextureFromImage( player_image );

	// initialize texture for the dots
	Image dots_image = { 0 };
	dots_image.format = 7;
    dots_image.width = img_data_dots.size_x;
    dots_image.height = img_data_dots.size_y;
    dots_image.data = img_data_dots.data;
    dots_image.mipmaps = 1;
    dots_texture = LoadTextureFromImage( dots_image );

	// initialize texture for the background
	Image background_image = { 0 };
	background_image.format = 7;
    background_image.width = img_data_background.size_x;
    background_image.height = img_data_background.size_y;
    background_image.data = img_data_background.data;
    background_image.mipmaps = 1;
    background_texture = LoadTextureFromImage( background_image );


	// initialize gameplay state
	state.current_level_idx = 0;
	state.current_level = &LDtkLevels[state.current_level_idx];

	// SHOULD CHECK LEVEL
	initialize_level( state.current_level );
}


static void initialize_level( const LDtkLevel* level )
{
	int i, j, k;
	const LDtkLayer* entities_layer;
	const LDtkEntity_player* player_entity;

	// level transitions
	state.nxt_level_timer = 120;
	state.finished_level = 0;
	state.skip_timer = 30;

	// state layers
	state.collision_layer = get_layer_by_name( "walls", state.current_level );
	state.hazards_layer = get_layer_by_name( "hazards", state.current_level );

	// search for player entity
	entities_layer = get_layer_by_name( "entities", level );

	// get player entity
	player_entity = &entities_layer->players[0];

	// set player properties
	state.p.cx = player_entity->px_x / entities_layer->gridSize;
	state.p.xr = 0.5f;//( float ) ( player_entity->px_x - state.p.cx * entities_layer->gridSize ) / (float ) entities_layer->gridSize;
	state.p.cy = player_entity->px_y / entities_layer->gridSize + 1;
	state.p.yr = 0.5f;//( float ) ( player_entity->px_y - state.p.cy * entities_layer->gridSize ) / (float ) entities_layer->gridSize;
	state.p.dir = 1;
	state.p.is_dead = 0;
	state.p.dx = 0.0f;
	state.p.dy = 0.0f;
	state.p.is_on_air = 0;
	

	// dots
	if( state.dots != NULL )
	{
		for( i = 0; i < state.ndots; i++ )
			free( state.dots[i].a );
		free( state.dots );
	}
	state.ndots = entities_layer->ndots;
	state.dots = ( Dot* ) calloc( ( size_t ) state.ndots, sizeof( Dot ) );
	
	for( i = 0; i < entities_layer->ndots; i++ )
	{
		// find dot corresponding to this value of i
		for( j = 0; j < entities_layer->ndots; j++ )
		{
			if( entities_layer->dots[j].counter == i )
			{
				state.dots[i].d = &entities_layer->dots[j];
				break;
			}
		}
		if( state.dots[i].d == NULL ) fprintf( stderr, "Error: Missing dot %d in level: %s\n", i, level->identifier );
		// initialize dot state
		state.dots[i].ds = DOT_INACTIVE;
		// create the animation for the dot
		int nanim = 3; // CAREFUL! THE NUMBER OF ANIMATIONS IS NOT KNWON
		state.dots[i].a = ( ASEpriteAnim* ) calloc( ( size_t ) nanim, sizeof( ASEpriteAnim ) ); 
		for( j = 0; j < nanim; j++ )
		{
			state.dots[i].a[j] = animations_dots[j];
			for( k = 0; k < animations_dots[j].nframes; k++ )
				state.dots[i].a[j].frames[k] = animations_dots[j].frames[k];
		}
	}
	state.dots[0].ds = DOT_IDLE;
}


static void update_gameplay_screen( void )
{
	float scale;
	PlayerMotionState player_motion = PLAYERMOTION_IDLE;

	if( state.finished_level == 0 )
	{
		// Update input map	
		update_input_map(&input);
		// Player controller
		player_motion = update_player_control( 
			&state.p,
			state.collision_layer,
			state.hazards_layer,
			input.btn_right - input.btn_left,
			input.btn_jump );
		if( player_motion == -1 )
		{
			// reset the level
			initialize_level( state.current_level );
			return;
		}
		// Dots controller
		if( state.p.is_dead == 0 )
		{
			DotAquired v = update_dots( &state );
			if( v == DOT_ACQUIRED_NOK )
			{
				// wrong dot... kill the player
				state.p.is_dead = 1;
				state.p.dead_timer = 120;
				return;
			}
			if( v == DOT_ACQUIRED_FINAL )
			{
				// finished! next level
				state.finished_level = 1;
				player_motion = PLAYERMOTION_DEAD;
			}
		}


		// SKIP LEVEL FOR DEBUG
		if( IsKeyDown( KEY_TAB ) )
		{
			state.skip_timer--;
			//printf("%d\n", state.skip_timer);
			if( state.skip_timer <= 0 )
			{
				state.finished_level = 1;
				player_motion = PLAYERMOTION_DEAD;
				state.nxt_level_timer = 6;
			}
		}
	}
	else
	{
		// ending level
		player_motion = PLAYERMOTION_DEAD;
		state.nxt_level_timer--;
		if( state.nxt_level_timer <= 0 )
		{
			
			if( LDtkLevels[state.current_level_idx].is_final )
			{
				// end the game
			}
			else
			{
				// next level
				state.current_level_idx += 1;
				state.current_level = &LDtkLevels[state.current_level_idx];
				initialize_level( state.current_level );
			}
		}
	}
	
	//-------------------------------
	// Draw everything
	//-------------------------------
	// Draw everything in the render texture, note this will not be rendered on screen, yet
	BeginTextureMode( target );
		ClearBackground( LIGHTGRAY );

		// draw background
		DrawTexturePro( background_texture,
			( Rectangle ){ 0, 0, 256, 256 }, 
			( Rectangle ){ ( int )( background_pos + 0.5f ), 0, 256, 256 },
			( Vector2 ){ 0, 0 }, 0.0f, WHITE );
		DrawTexturePro( background_texture,
			( Rectangle ){ 0, 0, 256, 256 }, 
			( Rectangle ){ ( int )( background_pos - 256.0f + 0.5f ), 0, 256, 256 },
			( Vector2 ){ 0, 0 }, 0.0f, WHITE );
		background_pos += 0.05f;
		if( background_pos > 256.0f ) background_pos -= 256.0f;

		draw_level( state.current_level, &tileset );
		draw_lines( &state );
		draw_dots( &state );
		draw_player( &state, player_motion );

		// debug drawing
		// float px = ( ( float ) state.p.cx + state.p.xr ) * 8.0f;
		// float py = ( ( float ) state.p.cy + state.p.yr ) * 8.0f;
		// DrawCircle( ( int ) ( px + 0.5 ), ( int ) ( py + 0.5 ), 2, RED );

		// int x = ( int )( ( state.p.cx + state.p.xr - 0.5 ) * 8 + 0.5 );
		// int y = ( int )( ( state.p.cy + state.p.yr - 0.5 ) * 8 + 0.5 );
		// Rectangle player_rect;
		// player_rect = ( Rectangle ){ x, y, 8, 8 };
		// DrawRectangleRec( player_rect, RED );

	EndTextureMode();

	// Draw everything on screen, after scaling
	scale = MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);
	BeginDrawing();
		// Draw gameplay texture
		ClearBackground( dark );
		// Draw render texture to screen, properly scaled
		DrawTexturePro( target.texture,
			(Rectangle){ 0.0f, 0.0f, ( float )target.texture.width, (float)-target.texture.height },
			(Rectangle){ ( GetScreenWidth() - ( ( float ) gameScreenWidth * scale ))*0.5f, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
						(float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
		// Draw UI - not scaled
	EndDrawing();
}




static void draw_player( GamePlayState* s, PlayerMotionState pm )
{
	const int gridsize = 8;

	// check if this is a new animation
	if( pm != s->pm )
	{
		s->pm = pm; // set the current animation
		reset_animation( &animations_player[s->pm] );
	}

	// draw the current animation
	int x, y;
	x = ( int )( gridsize * ( ( float ) s->p.cx + s->p.xr ) + 0.5 - 8.0 );
	y = ( int )( gridsize * ( ( float ) s->p.cy + s->p.yr ) + 0.5 - 9.0 );
	draw_frame( ( float ) x, ( float ) y, &animations_player[s->pm].frames[animations_player[s->pm].idx], &player_texture, 1.0f * s->p.dir, 1.0f );

	// update the current animation
	update_animation( &animations_player[s->pm], ( int )( GetFrameTime() * 1000.0f + 0.5 ) );
	

}


static void draw_dots( GamePlayState* s )
{
	int i;
	float x, y;

	// cycle dots
	for( i = 0; i < s->ndots; i++ )
	{
		// draw the current frame
		x = ( float ) s->dots[i].d->px_x;
		y = ( float ) s->dots[i].d->px_y;
		// draw frame
		draw_frame( x, y, &s->dots[i].a[s->dots[i].ds].frames[s->dots[i].a[s->dots[i].ds].idx], &dots_texture, 1.0f, 1.0f );
		// update the current animation
		update_animation( &s->dots[i].a[s->dots[i].ds], ( int )( GetFrameTime() * 1000.0f + 0.5 ) );
	}
	
}



static DotAquired update_dots( GamePlayState* s )
{
	int i;
	float dx, dy;
	float px, py;
	
	// player position
	px = ( ( float ) s->p.cx + s->p.xr ) * 8.0f; 
	py = ( ( float ) s->p.cy + s->p.yr ) * 8.0f;

	for( i = 0; i < s->ndots; i++ )
	{
		// check x/y distance to player
		dx = ( float )( s->dots[i].d->px_x + 4 ) - px;
		dy = ( float )( s->dots[i].d->px_y + 4 ) - py;
		if( s->dots[i].ds == DOT_IDLE )
		{
			if( ( ( dx * dx ) + ( dy * dy ) ) < ( 6 * 6 ) )
			{
				s->dots[i].ds = DOT_ACTIVE;
				if( i < ( s->ndots - 1 ) ) s->dots[i + 1].ds = DOT_IDLE;
				s->dots_timer = 60;
				if( i == s->ndots - 1 )
					return DOT_ACQUIRED_FINAL;
				return DOT_ACQUIRED_OK;
			}
		}
		else if( s->dots[i].ds == DOT_INACTIVE )
		{
			if( ( ( dx * dx ) + ( dy * dy ) ) < ( 5 * 5 ) )
			{
				return DOT_ACQUIRED_NOK;
			}
		}
	}
	return DOT_NOT_ACQUIRED;
}





static void draw_lines( GamePlayState* s )
{
	int i;

	for( i = 1; i < s->ndots; i++ )
	{
		if( s->dots[i].ds == DOT_ACTIVE )
		{
			DrawLineEx
			(
				( Vector2 ){ s->dots[i-1].d->px_x + 4, s->dots[i-1].d->px_y + 4 },
				( Vector2 ){ s->dots[i].d->px_x + 4, s->dots[i].d->px_y + 4 },
				3, dark
			);
		}
	}
}