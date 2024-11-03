#include <raylib.h>
#include "aseprite_tools.h"
#include <stdio.h>

void draw_aseprite_frame( float x, float y, ASEpriteFrame* frame, Texture2D* texture, float scale_x, float scale_y )
{
	// printf( "%.1f, %.1f\n", scale_x, scale_y);
	DrawTexturePro( *texture,
		( Rectangle ){ ( float ) frame->x, ( float ) frame->y, ( float ) frame->w * scale_x, ( float ) frame->h * scale_y },
		( Rectangle ){ x, y, ( float ) frame->w, ( float ) frame->h },
		( Vector2 ){ 0.0f, 0.0f }, 0.0f, WHITE );
}

void draw_aseprite_cur_frame( float x, float y, ASEpriteAnim* anim, Texture2D* texture, float scale_x, float scale_y )
{
	draw_aseprite_frame( x, y, &anim->frames[anim->idx], texture, scale_x, scale_y );
}

void update_aseprite_animation( ASEpriteAnim* anim, int dt_ms )
{
	anim->t_ms -= dt_ms;
	if( anim->t_ms <= 0 )
	{
		if( anim->rep )
		{
			anim->idx = ( anim->idx + 1 ) % anim->nframes;
		}
		else
		{
			anim->idx++;
			if( anim->idx > anim->nframes - 1 ) anim->idx = anim->nframes - 1;

		}
		anim->t_ms += anim->frames[anim->idx].duration_ms;
	}
}

void reset_aseprite_animation( ASEpriteAnim* anim )
{
	//printf( "Resetting animation: %s\n", anim->name );
	anim->idx = 0;
	anim->t_ms = anim->frames[anim->idx].duration_ms;
}