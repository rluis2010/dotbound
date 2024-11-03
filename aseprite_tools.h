#ifndef ASEPRITE_TOOLS_H
#define ASEPRITE_TOOLS_H

typedef struct
{
	int x; int y;
	int w; int h;
	int duration_ms;
} ASEpriteFrame;

typedef struct
{
	char* name;
	int from; int to;
	int dir;
	int rep;
	int nframes;
	ASEpriteFrame* frames;
	int idx; // frame index
	int t_ms;
} ASEpriteAnim;


void draw_aseprite_frame( float x, float y, ASEpriteFrame* frame, Texture2D* texture, float scale_x, float scale_y );
void draw_aseprite_cur_frame( float x, float y, ASEpriteAnim* anim, Texture2D* texture, float scale_x, float scale_y );
void update_aseprite_animation( ASEpriteAnim* anim, int dt_ms );
void reset_aseprite_animation( ASEpriteAnim* anim );

#endif