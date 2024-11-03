#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <raymath.h>

#include "defs.h"
#include "ldtk_tools.h"
#include "player_controller.h"
#include "aseprite_tools.h"

#include "tilemap.h"

#include "player_anim.h"
#include "dots_anim.h"
#include "explosion_anim.h"


// Images
#include "tileset_img.h"
#include "player_img.h"
#include "dots_img.h"
#include "background_img.h"
#include "explosion_img.h"

// Music
#ifndef USE_EXTERNAL_RESOURCE_FILES
#include "tunnels.h"
#endif
// SFX
#include <jump.h>
#include <dot.h>
#include <hurt.h>


#define PLAYER_X ( ( ( float ) player.cx + player.xr ) * 8.0f + 4.0f )
#define PLAYER_Y ( ( ( float ) player.cy + player.yr ) * 8.0f + 4.0f )
#define STARTING_LEVEL 0

//-----------------------------------------------------------------------------------
// Type definitions
//-----------------------------------------------------------------------------------
typedef enum
{
	GAMEPLAY_SCREEN_INITIALIZE_LEVEL,
	GAMEPLAY_SCREEN_START_LEVEL,
	GAMEPLAY_SCREEN_RUN_LEVEL,
	GAMEPLAY_SCREEN_END_LEVEL,
} GameplayScreenState;

typedef enum
{
	DOT_IDLE = 0,
	DOT_WAITING,
	DOT_SELECTED
} DotState;

typedef struct
{
	ASEpriteAnim* animation;
	DotState state;
	const LDtkEntity_dot* entity;
	int anim_cur;
	int anim_nxt;
	int has_explosion;
} Dot;

//-----------------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------------
// General purpose
static RenderTexture2D screen_target;
static GameplayScreenState gameplayscreen_state;
static int current_level_idx = 2;
static int next_level_idx = 2;
static PlayerController player;

// Variables for the start state
static float start_level_circle_radius;

// Textures
static Texture2D tileset_tex;
static Texture2D player_tex;
static Texture2D dots_tex;
static Texture2D background_tex;
static Texture2D explosion_tex;

// Music
static Music game_music;

// SFX
static Sound jump_sfx;
static Sound dot_sfx;
static Sound hurt_sfx;

// Dots
static int total_ndots;
static Dot* dots;
static int ndots;

//-----------------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------------
static void state_initialize_level( void );
static void state_start_level( void );
static int state_run_level( void );
static int state_end_level( void );

static Texture2D load_texture_from_header( img_data* image_data );
static void draw_moving_background_tex( void );
static void draw_player( void );
static void draw_dots( void );
static void draw_lines( void );
static void dots_game_logic( void );

//-----------------------------------------------------------------------------------
// Load all the resources required for the gameplay screen
//-----------------------------------------------------------------------------------
void gameplay_screen_initialize( void )
{
	int i, j, k;
	const LDtkLayer* entities_layer;

	// initialize render texture (to scale to window size )
	screen_target = LoadRenderTexture( gameScreenWidth, gameScreenHeight);
	SetTextureFilter( screen_target.texture, TEXTURE_FILTER_POINT );

	// Load textures
	tileset_tex = load_texture_from_header( &img_data_tileset );
	player_tex = load_texture_from_header( &img_data_player );
	dots_tex = load_texture_from_header( &img_data_dots );
	background_tex = load_texture_from_header( &img_data_background );
	explosion_tex = load_texture_from_header( &img_data_explosion );

	// Gameplay and level
	gameplayscreen_state = GAMEPLAY_SCREEN_INITIALIZE_LEVEL;
	current_level_idx = STARTING_LEVEL;
	next_level_idx = STARTING_LEVEL;

	// Compute how many dots are required in total
	total_ndots = 0;
	for( i = 0; i < LDtkLevels[0].nlevels; i++ )
	{
		entities_layer = get_layer_by_name( "entities", &LDtkLevels[i] );
		if( entities_layer->ndots > total_ndots ) total_ndots = entities_layer->ndots;
	}
	// Allocate memory for the dots
	int nanim = 3; // TODO: CAREFUL!!! THIS IS HARDCODED
	dots = ( Dot* ) calloc( ( size_t ) total_ndots, sizeof( Dot ) );
	for( i = 0; i < total_ndots; i++ )
	{
		dots[i].animation = ( ASEpriteAnim* ) calloc( ( size_t ) nanim, sizeof( ASEpriteAnim ) );
		for( j = 0; j < nanim; j++ )
		{
			dots[i].animation[j] = animations_dots[j];
			for( k = 0; k < animations_dots[j].nframes; k++ )
				dots[i].animation[j].frames[k] = animations_dots[j].frames[k];
		}
	}
	
	// Music
#ifndef USE_EXTERNAL_RESOURCE_FILES
	game_music = LoadMusicStreamFromMemory( ".ogg", __assets_music_tunnels_ogg, __assets_music_tunnels_ogg_len );
#else
	game_music = LoadMusicStream( "assets/music/tunnels.ogg" );
#endif
	PlayMusicStream( game_music );
	SetMusicVolume( game_music, 0.25f );


	// SFX
	Wave jump_sfx_wave = LoadWaveFromMemory( ".wav", __assets_sfx_jump_wav, __assets_sfx_jump_wav_len );
	jump_sfx = LoadSoundFromWave( jump_sfx_wave );
	SetSoundVolume( jump_sfx, 0.15f );
	UnloadWave( jump_sfx_wave );

	Wave dot_sfx_wave = LoadWaveFromMemory( ".wav", __assets_sfx_dot_wav, __assets_sfx_dot_wav_len );
	dot_sfx = LoadSoundFromWave( dot_sfx_wave );
	SetSoundVolume( dot_sfx, 0.2f );
	UnloadWave( dot_sfx_wave );

	Wave hurt_sfx_wave = LoadWaveFromMemory( ".wav", __assets_sfx_hurt_wav, __assets_sfx_hurt_wav_len );
	hurt_sfx = LoadSoundFromWave( hurt_sfx_wave );
	SetSoundVolume( hurt_sfx, 0.25f );
	UnloadWave( hurt_sfx_wave );
}

//-----------------------------------------------------------------------------------
// Clean up all the resources used in the gameplay screen
//-----------------------------------------------------------------------------------
void gameplay_screen_terminate( void )
{
	int i;

	// Free dot animations
	if( dots != NULL )
	{
		for( i = 0; i < total_ndots; i++ )
			free( dots[i].animation );
		free( dots );
	}
	// Unload textures
	UnloadTexture( tileset_tex );
	UnloadTexture( player_tex );
	UnloadTexture( dots_tex );
	UnloadTexture( background_tex );
	UnloadTexture( explosion_tex );
	UnloadRenderTexture( screen_target );

	StopMusicStream( game_music );
	UnloadMusicStream( game_music );

	UnloadSound( jump_sfx );
}

//-----------------------------------------------------------------------------------
// Run the gameplay screen every frame
//-----------------------------------------------------------------------------------
GameScreen gameplay_screen_run( void )
{
	int res;
	// gameplay screen states machine
	switch( gameplayscreen_state )
	{
		case GAMEPLAY_SCREEN_INITIALIZE_LEVEL:
			state_initialize_level();
			break;
		case GAMEPLAY_SCREEN_START_LEVEL:
			state_start_level();
			break;
		case GAMEPLAY_SCREEN_RUN_LEVEL:
			res = state_run_level();
			//if( res ) return SCREEN_ENDING;
			break;
		case GAMEPLAY_SCREEN_END_LEVEL:
			res = state_end_level();
			if( res ) return SCREEN_ENDING;
			break;
	}


	// Draw everything on screen, after scaling
	float scale = MIN((float)GetScreenWidth()/gameScreenWidth, (float)GetScreenHeight()/gameScreenHeight);
	BeginDrawing();
		// Draw render texture to screen, properly scaled
		ClearBackground( CANDY_DARK );
		DrawTexturePro( screen_target.texture,
			(Rectangle){ 0.0f, 0.0f, ( float )screen_target.texture.width, (float)-screen_target.texture.height },
			(Rectangle){ ( GetScreenWidth() - ( ( float ) gameScreenWidth * scale ))*0.5f, (GetScreenHeight() - ((float)gameScreenHeight*scale))*0.5f,
						(float)gameScreenWidth*scale, (float)gameScreenHeight*scale }, (Vector2){ 0, 0 }, 0.0f, WHITE);
		// Draw UI - not scaled
	EndDrawing();

	// Music
	UpdateMusicStream( game_music );

	return SCREEN_NONE;
}




//-----------------------------------------------------------------------------------
// Initialize level state. Only runs for a single frame
//-----------------------------------------------------------------------------------
static void state_initialize_level( void )
{
	int i, j;
	const LDtkLayer* entities_layer;
	const LDtkEntity_player* player_tex_entity;

	printf( "Initializing Level %d\n", current_level_idx );

	// isolate entities layer
	entities_layer = get_layer_by_name( "entities", &LDtkLevels[current_level_idx] );
	
	// get player_tex entity
	player_tex_entity = &entities_layer->players[0]; // there should be only one player_tex

	// set player_tex properties
	player.cx = player_tex_entity->px_x / entities_layer->gridSize;
	player.xr = 0.5f;
	player.cy = player_tex_entity->px_y / entities_layer->gridSize + 1;
	player.yr = 0.5f;
	player.dir_nxt = 1;
	player.anim_nxt = 0;
	player.collision_layer = get_layer_by_name( "walls", &LDtkLevels[current_level_idx] );
	player.hazards_layer = get_layer_by_name( "hazards", &LDtkLevels[current_level_idx] );
	player.hurt_sfx = &hurt_sfx;
	player_initialize( &player );
	

	// next state to start the level
	start_level_circle_radius = 0.0f;
	reset_aseprite_animation( &animations_explosion[0] );
	reset_aseprite_animation( &animations_player[0] );
	gameplayscreen_state = GAMEPLAY_SCREEN_START_LEVEL;

	// prepare required number of dots
	ndots = entities_layer->ndots;
	for( i = 0; i < ndots; i++ )
	{
		reset_aseprite_animation( &dots[i].animation[0] );
		dots[i].state = ( i == 0 ) ? DOT_WAITING : DOT_IDLE;
		dots[i].anim_nxt = ( i == 0 ) ? 1 : 0;
		dots[i].has_explosion = 0;
		// search for entity with this dot number
		dots[i].entity = NULL;
		for( j = 0; j < ndots; j++ )
			if( entities_layer->dots[j].counter == i ) dots[i].entity = &entities_layer->dots[j];
		if( dots[i].entity == NULL ) fprintf( stderr, "Unable to find dot entity with number %d on level %s\n", i, LDtkLevels[current_level_idx].identifier );
	}


	// clear screen
	BeginTextureMode( screen_target );
		ClearBackground( CANDY_DARK );
	EndTextureMode();
	
}


//-----------------------------------------------------------------------------------
// Start state. Used for the opening circle
//-----------------------------------------------------------------------------------
static void state_start_level( void )
{
	// Draw things in the render texture
	BeginTextureMode( screen_target );
		draw_moving_background_tex();
		draw_ldtk_level( &LDtkLevels[current_level_idx], &tileset_tex );
		draw_lines();
		draw_dots();

		if( start_level_circle_radius < 1.0f )
		{
			DrawRing( ( Vector2 ){ 128, 128 }, Lerp( -256.0f, 256.0f, start_level_circle_radius ), 512.0f, 0.0f, 360.0f, 64, CANDY_DARK );
			start_level_circle_radius += 0.0125f;
		}
		else
		{
			if( animations_explosion[0].idx >= 4 )
			{
				gameplayscreen_state = GAMEPLAY_SCREEN_RUN_LEVEL; // move to run state
				//reset_aseprite_animation( &animations_explosion[0] );
			}
			// draw player and explosion
			draw_player();
			draw_aseprite_cur_frame( PLAYER_X - 12.0f, PLAYER_Y - 13.0f, &animations_explosion[0], &explosion_tex, 1.0f, 1.0f );
			update_aseprite_animation( &animations_explosion[0], GetFrameTime() * 1000.0f );
			
		}
	EndTextureMode();
}

//-----------------------------------------------------------------------------------
// Run state. Used for the actual game
//-----------------------------------------------------------------------------------
static int state_run_level( void )
{
	//int i;
	//int new_player_anim;

	// Game logic
	player_fsm( &player );

	// Player interactions with dots
	dots_game_logic();

	// Check if all dots are captured
	if( dots[ndots - 1].state == DOT_SELECTED )
	{
		// move to next level
		if( !LDtkLevels[current_level_idx].is_final )
		{
			next_level_idx = current_level_idx + 1;
			start_level_circle_radius = 1.0f;
			gameplayscreen_state = GAMEPLAY_SCREEN_END_LEVEL;
		}
		else
		{
			// end the game
			start_level_circle_radius = 1.0f;
			gameplayscreen_state = GAMEPLAY_SCREEN_END_LEVEL;
		}
	}

	// Check if the player is dead
	if( player.state_cur == PLAYERSTATE_DEAD && player.dead_timer <= 0 )
	{
		// restart the level
		start_level_circle_radius = 1.0f;
		gameplayscreen_state = GAMEPLAY_SCREEN_END_LEVEL;
	}

	// Draw things in the render texture
	BeginTextureMode( screen_target );
		draw_moving_background_tex();
		draw_ldtk_level( &LDtkLevels[current_level_idx], &tileset_tex );
		draw_lines();
		draw_dots();
		draw_player();
	EndTextureMode();
	return 0;
}


//-----------------------------------------------------------------------------------
// End state. Used for the closing circle
//-----------------------------------------------------------------------------------
static int state_end_level( void )
{
	// Draw things in the render texture
	BeginTextureMode( screen_target );
		ClearBackground( CANDY_DARK );
		if( start_level_circle_radius > 0.0f )
		{
			
			start_level_circle_radius -= 0.025f;
			if( start_level_circle_radius < 0.0f ) start_level_circle_radius = 0.0f;
			draw_moving_background_tex();
			draw_ldtk_level( &LDtkLevels[current_level_idx], &tileset_tex );
			draw_lines();
			draw_dots();
			if( player.state_cur != PLAYERSTATE_DEAD )
				draw_player();
			DrawRing( ( Vector2 ){ 128, 128 }, Lerp( 0.0f, 256.0f, start_level_circle_radius ), 512.0f, 0.0f, 360.0f, 64, CANDY_DARK );
		}
		else
		{

			if( LDtkLevels[current_level_idx].is_final && player.state_cur != PLAYERSTATE_DEAD )
			{
				return 1;
			}
			else
			{
				current_level_idx = next_level_idx;
				gameplayscreen_state = GAMEPLAY_SCREEN_INITIALIZE_LEVEL; // move to initialization state
			}
		}
	EndTextureMode();
	return 0;
}





//-----------------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------------
static Texture2D load_texture_from_header( img_data* image_data )
{
	Image image = { 0 };
	image.format = 7;
    image.width = image_data->size_x;
    image.height = image_data->size_y;
    image.data = image_data->data;
    image.mipmaps = 1;
    return LoadTextureFromImage( image );
}


static float background_tex_pos;
static void draw_moving_background_tex( void )
{
	DrawTexturePro( background_tex,
		( Rectangle ){ 0, 0, 256, 256 }, 
		( Rectangle ){ ( int )( background_tex_pos + 0.5f ), 0, 256, 256 },
		( Vector2 ){ 0, 0 }, 0.0f, WHITE );
	DrawTexturePro( background_tex,
		( Rectangle ){ 0, 0, 256, 256 }, 
		( Rectangle ){ ( int )( background_tex_pos - 256.0f + 0.5f ), 0, 256, 256 },
		( Vector2 ){ 0, 0 }, 0.0f, WHITE );
	background_tex_pos += 0.05f;
	if( background_tex_pos > 256.0f ) background_tex_pos -= 256.0f;
}


static void draw_player( void )
{
	if( player.anim_cur != player.anim_nxt )
	{
		player.anim_cur = player.anim_nxt;
		reset_aseprite_animation( &animations_player[player.anim_cur] );

		// SFX that is dependent on the animations
		if( player.anim_cur == 2 )
		{
			SetSoundPitch( jump_sfx, 1.0 + ( ( float ) GetRandomValue( -1, 1 ) ) * 0.2f );
			PlaySound( jump_sfx );
		}
	}

	// draw the current frame
	draw_aseprite_cur_frame
	(
		PLAYER_X - 12.0f, PLAYER_Y - 13.0f,
		&animations_player[player.anim_cur], &player_tex,
		1.0f * player.dir_cur, 1.0f
	);

	// update the current animation
	update_aseprite_animation( &animations_player[player.anim_cur], ( int )( GetFrameTime() * 1000.0f + 0.5 ) );
}


static void draw_dots( void )
{
	int i;
	for( i = 0; i < ndots; i++ )
	{
		if( dots[i].anim_cur != dots[i].anim_nxt )
		{
			dots[i].anim_cur = dots[i].anim_nxt;
			reset_aseprite_animation( &dots[i].animation[dots[i].anim_cur] );
		}

		draw_aseprite_cur_frame
		(
			dots[i].entity->px_x, dots[i].entity->px_y,
			&dots[i].animation[dots[i].anim_cur], &dots_tex,
			1.0f, 1.0f
		);

		update_aseprite_animation( &dots[i].animation[dots[i].anim_cur], ( int )( GetFrameTime() * 1000.0f + 0.5 ) );

		if( dots[i].has_explosion )
		{
			if( animations_explosion[0].idx >= 4 )
				dots[i].has_explosion = 0;
			// draw explosion
			draw_aseprite_cur_frame( dots[i].entity->px_x - 4, dots[i].entity->px_y - 4, &animations_explosion[0], &explosion_tex, 1.0f, 1.0f );
			update_aseprite_animation( &animations_explosion[0], GetFrameTime() * 1000.0f );
		}
	}
}

static void draw_lines( void )
{
	int i;

	for( i = 1; i < ndots; i++ )
	{
		if( dots[i].state == DOT_SELECTED )
		{
			DrawLineEx
			(
				( Vector2 ){ dots[i - 1].entity->px_x + 4, dots[i - 1].entity->px_y + 4 },
				( Vector2 ){ dots[i].entity->px_x + 4, dots[i].entity->px_y + 4 },
				3, CANDY_DARK
			);
		}
	}
}


static void dots_game_logic( void )
{
	int i;
	if( player.state_cur != PLAYERSTATE_DEAD )
		for( i = 0; i < ndots; i ++ )
		{
			switch( dots[i].state )
			{
				case DOT_SELECTED:
					break;
				case DOT_IDLE:
					if( player_check_collision_with_dot( dots[i].entity, &player, 1 ) )
					{
						player.state_nxt = PLAYERSTATE_DEAD;
						player.anim_nxt = 4;
						player.dead_timer  = 60;
						PlaySound( hurt_sfx );
					}
					break;
				case DOT_WAITING:
					if( player_check_collision_with_dot( dots[i].entity, &player, 0 ) )
					{
						dots[i].state = DOT_SELECTED;
						dots[i].anim_nxt = 2;
						dots[i].has_explosion = 1;
						reset_aseprite_animation( &animations_explosion[0] );
						if( i < ( ndots - 1 ) )
						{
							dots[i + 1].state = DOT_WAITING;
							dots[i + 1].anim_nxt = 1;
						}
						PlaySound( dot_sfx );
					}
					break;
			}
		}
}










