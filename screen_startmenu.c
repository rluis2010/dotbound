#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include "screen_startmenu.h"

#include "defs.h"
#include "title_img.h"
#include "background_img.h"

#ifndef USE_EXTERNAL_RESOURCE_FILES
#include "smody_start.h"
#endif

//-----------------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------------
// General purpose
static RenderTexture2D screen_target;
static int start_timer = 30;

// Textures
static Texture2D title_tex;
static Texture2D background_tex;
static int flash_timer = 0;
// Music
static Music start_music;

//-----------------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------------
static Texture2D load_texture_from_header( img_data* image_data );
static void draw_moving_background_tex( void );


//-----------------------------------------------------------------------------------
// Load all the resources required for the startmenu screen
//-----------------------------------------------------------------------------------
void startmenu_screen_initialize( void )
{
	// initialize render texture (to scale to window size )
	screen_target = LoadRenderTexture( gameScreenWidth, gameScreenHeight);
	SetTextureFilter( screen_target.texture, TEXTURE_FILTER_POINT );

	// Textures
	background_tex = load_texture_from_header( &img_data_background );
	title_tex = load_texture_from_header( &img_data_title );

	// Start music
#ifndef USE_EXTERNAL_RESOURCE_FILES
	start_music = LoadMusicStreamFromMemory( ".ogg", __assets_music_smody_start_ogg, __assets_music_smody_start_ogg_len );
#else
	start_music = LoadMusicStream( "assets/music/smody_start.ogg" );
#endif
	PlayMusicStream( start_music );
	SetMusicVolume( start_music, 0.25f );
}

//-----------------------------------------------------------------------------------
// Clean up all the resources used in the startmenu screen
//-----------------------------------------------------------------------------------
void startmenu_screen_terminate( void )
{
	UnloadTexture( background_tex );
	UnloadTexture( title_tex );
	UnloadRenderTexture( screen_target );

	StopMusicStream( start_music );
	UnloadMusicStream( start_music );
}

//-----------------------------------------------------------------------------------
// Run the startmenu screen every frame
//-----------------------------------------------------------------------------------
GameScreen startmenu_screen_run( void )
{
	// Logic
	if( start_timer > 0 )
	{
		start_timer--;
	}
	else
	{
		if( GetKeyPressed() != 0 )
		{
			// Start Game
			return SCREEN_GAMEPLAY;
		}
	}

	// Draw stuff on texture
	BeginTextureMode( screen_target );
		// background sky
		draw_moving_background_tex();

		// title
		DrawTexturePro( title_tex,
			( Rectangle ){ 0, 0, 256, 256 }, 
			( Rectangle ){ 0, 0, 256, 256 },
			( Vector2 ){ 0, 0 }, 0.0f, WHITE );
		
		// start message
		flash_timer++;
		if( flash_timer < 60 )
		{
			char* text = "Press Any Key";
			int text_size = MeasureText( text, 16 );
			DrawText( text, 128-text_size / 2, 128 - 16, 16, CANDY_DARK );
		}
		else if( flash_timer > 90 )
		{
			flash_timer = 0;
		}

		// Credits
		DrawText( "Music by:\n  Congubongus\n  (https://soundcloud.com/congus-bongus)\n  Smody\n  (https://smody.itch.io/)", 16, 128 + 56, 6, GetColor( 0xfbf7b6ff ) );

	EndTextureMode();

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
	UpdateMusicStream( start_music );

	return SCREEN_NONE;
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

static float background_tex_pos = 0.0f;
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