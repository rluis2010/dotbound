#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include "tilemap.h"

typedef enum
{
	PLAYERMOTION_IDLE,
	PLAYERMOTION_RUN,
	PLAYERMOTION_JUMP,
	PLAYERMOTION_FALL,
	PLAYERMOTION_DEAD,
} PlayerMotionState;

typedef enum
{
	PLAYERSTATE_FLOOR = 0,
	PLAYERSTATE_AIR,
	PLAYERSTATE_WALL,
	PLAYERSTATE_DEAD,
} PlayerState;

// definitions
typedef struct
{
	int cx; int cy;
	float xr; float yr;
	float dx; float dy;
	int dir_nxt;
	int dir_cur;
	// int is_on_air;
	int is_dead;
	int coyote_timer;
	int jump_buffer;
	int dead_timer;
	int wall_timer;
	int wall_dir_timer;
	int anim_cur;
	int anim_nxt;
	const LDtkLayer* collision_layer;
	const LDtkLayer* hazards_layer;
	Sound* hurt_sfx;

	PlayerState state_nxt;
	PlayerState state_cur;
} PlayerController;

// function declarations
void player_initialize( PlayerController* p );
void player_fsm( PlayerController* p );
int player_check_collision_with_dot( const LDtkEntity_dot* dot_entity, PlayerController* p, int for_damage );






#endif