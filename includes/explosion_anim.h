#ifndef EXPLOSION_JSON_H
#define EXPLOSION_JSON_H



#include "../aseprite_tools.h"

static const ASEpriteFrame frames_explosion[] =
{
	{ .x = 0, .y = 0, .w = 16, .h = 16, .duration_ms = 100 },
	{ .x = 16, .y = 0, .w = 16, .h = 16, .duration_ms = 100 },
	{ .x = 32, .y = 0, .w = 16, .h = 16, .duration_ms = 100 },
	{ .x = 0, .y = 16, .w = 16, .h = 16, .duration_ms = 100 },
	{ .x = 16, .y = 16, .w = 16, .h = 16, .duration_ms = 100 },
};

static ASEpriteAnim animations_explosion[] =
{
	{
		.name = "explosion",
		.from = 0, .to = 4, .dir = 1,
		.rep = 0,
		.nframes = 5,
		.frames = ( ASEpriteFrame[] )
		{ frames_explosion[0], frames_explosion[1], frames_explosion[2], frames_explosion[3], frames_explosion[4], },
	},
};

#endif
