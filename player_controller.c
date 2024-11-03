#include <raylib.h>
#include <raymath.h>
#include "player_controller.h"
#include "ldtk_tools.h"
#include "input_map.h"

#include <stdio.h>





#define DIR ( input.btn_right - input.btn_left )

//-----------------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------------
const float max_velx = 0.125f;//0.2f;
const float gravity = 0.045f;//0.06f;
const float term_vely = 0.35f;//0.8f;
const float jump_vel = 0.6f;//0.78f;
const float air_accel = 0.166f;
const int coyote_frames = 6;
const int jump_buffer_frames = 9;
const int wall_jump_margin = 6;//10;
const float wall_jump_vel_ratio = 1.5f;

//-----------------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------------
static InputMap input;


//-----------------------------------------------------------------------------------
// Local function definitions
//-----------------------------------------------------------------------------------
static void state_player_floor( PlayerController* p );
static void state_player_air( PlayerController* p );
static void state_player_wall( PlayerController* p );
static void state_player_dead( PlayerController* p );
static void jump( PlayerController* p );
static int check_hazards( PlayerController* p );

//-----------------------------------------------------------------------------------
// Initialize the player
//-----------------------------------------------------------------------------------
void player_initialize( PlayerController* p )
{
	p->dx = 0.0f;
	p->dy = 0.0f;
	p->dir_cur = 1;
	// p->is_on_air = 0;
	p->is_dead = 0;
	p->coyote_timer = 0;
	p->dead_timer = 0;
	p->wall_timer = 0;
	p->state_cur = -1;
	p->state_nxt = PLAYERSTATE_FLOOR;
}

//-----------------------------------------------------------------------------------
// Player FSM
//-----------------------------------------------------------------------------------
void player_fsm( PlayerController* p )
{
	// some common definitions
	if( p->dir_cur != p->dir_nxt )
		p->dir_cur = p->dir_nxt;
	
	if( p->state_nxt != p->state_cur )
	{
		p->state_cur = p->state_nxt;
		// TODO: code to initialize state
	}

	switch( p->state_cur )
	{
		case PLAYERSTATE_FLOOR:
			state_player_floor( p );
			break;
		case PLAYERSTATE_AIR:
			state_player_air( p );
			break;
		case PLAYERSTATE_WALL:
			state_player_wall( p );
			break;
		case PLAYERSTATE_DEAD:
			state_player_dead( p );
			break;
	}
}

//-----------------------------------------------------------------------------------
// State Floor
//-----------------------------------------------------------------------------------
static void state_player_floor( PlayerController* p )
{
	int dir;

	// Update player input
	update_input_map(&input);

	// update direction
	dir = DIR;
	if( dir != 0 ) p->dir_nxt = dir;

	// reset coyote timer
	p->coyote_timer = coyote_frames;
	// reset wall timer
	p->wall_timer = 0;


	// move player horizontally
	p->dx = max_velx * (float) dir;
	p->xr += p->dx;

	// update animation
	p->anim_nxt = ( dir == 0 ) ? 0 : 1;

	
	// check collisions
	if( has_collision( p->cx + 1, p->cy, p->collision_layer, 1 ) && p->xr >= 0.5 )
	{
		p->xr = 0.5;
		p->dx = 0.0;
	}
	if( has_collision( p->cx - 1, p->cy, p->collision_layer, 1 ) && p->xr <= 0.5 )
	{
		p->xr = 0.5;
		p->dx = 0.0;
	}
	// align grid
	while( p->xr > 1.0f )
	{
		p->xr--;
		p->cx++;
	}
	while( p->xr < 0.0f )
	{
		p->xr++;
		p->cx--;
	}

	// move player vertically
	p->dy += gravity;
	if( p->dy > term_vely ) p->dy = term_vely;
	p->yr += p->dy;
	
	// check collisions
	if( has_collision( p->cx, p->cy - 1, p->collision_layer, 1 ) && p->yr <= 0.3f )
	{
		p->yr = 0.3;
		p->dy = 0.0;
		
	}
	if( ( has_collision( p->cx, p->cy + 1, p->collision_layer, 1 ) || has_collision( p->cx, p->cy + 1, p->collision_layer, 2 ) ) && p->yr > 0.5f )
	{
		p->yr = 0.5;
		p->dy = 0.0;
		// p->is_on_air = 0;
	}
	else
	{
		// falling
		p->coyote_timer = coyote_frames;
		p->state_nxt = PLAYERSTATE_AIR;
	}
	// align grid
	while( p->yr > 0.5f )
	{
		p->yr--;
		p->cy++;
	}
	while( p->yr < -0.5f )
	{
		p->yr++;
		p->cy--;
	}

	// check for jump
	if( input.btn_jump )
	{
		jump( p );
	}

	// Death
	check_hazards( p );
}





//-----------------------------------------------------------------------------------
// State Air
//-----------------------------------------------------------------------------------
static void state_player_air( PlayerController* p )
{
	int dir = 0;

	// Update player input
	update_input_map(&input);

	// update animations
	p->anim_nxt = ( p->dy < 0.0f ) ? 2 : 3;

	// update direction
	if( p->wall_timer <= 0 )
	{
		dir = DIR;
		p->dx = Lerp( p->dx, max_velx * ( float ) dir, air_accel );
	}
	else
	{
		p->wall_timer--;
	}
	if( dir != 0 ) p->dir_nxt = dir;

	

	// move player horizontally on the air
	p->xr += p->dx;
	
	// check collisions
	if( has_collision( p->cx + 1, p->cy, p->collision_layer, 1 ) && p->xr >= 0.5 )
	{
		p->xr = 0.5;
		p->dx = 0.0;
		if( dir > 0 )
		{
			p->dx = 0.0f;
			p->dy = 0.0f;
			p->anim_nxt = 5;
			p->state_nxt = PLAYERSTATE_WALL;
			p->wall_timer = 30;
		}
	}
	if( has_collision( p->cx - 1, p->cy, p->collision_layer, 1 ) && p->xr <= 0.5 )
	{
		p->xr = 0.5;
		p->dx = 0.0;
		if( dir < 0 )
		{
			p->dx = 0.0f;
			p->dy = 0.0f;
			p->anim_nxt = 5;
			p->state_nxt = PLAYERSTATE_WALL;
			p->wall_timer = 30;
			p->wall_dir_timer = 6;
		}
	}
	// align grid
	while( p->xr > 1.0f )
	{
		p->xr--;
		p->cx++;
	}
	while( p->xr < 0.0f )
	{
		p->xr++;
		p->cx--;
	}

	// move player vertically
	p->dy += gravity;
	if( p->dy > term_vely ) p->dy = term_vely;
	p->yr += p->dy;
	
	// check collisions
	if( has_collision( p->cx, p->cy - 1, p->collision_layer, 1 ) && p->yr <= 0.3f )
	{
		p->yr = 0.3;
		p->dy = 0.0;
		
	}
	if( ( has_collision( p->cx, p->cy + 1, p->collision_layer, 1 ) || has_collision( p->cx, p->cy + 1, p->collision_layer, 2 ) ) && p->yr > 0.5f ) // includes platforms
	{
		p->yr = 0.5;
		p->dy = 0.0;
		if( p->jump_buffer > 0 )
		{
			jump( p );
		}
		else
		{
			p->dy = 0.0f;
			p->state_nxt = PLAYERSTATE_FLOOR;
		}
	}

	// align grid
	while( p->yr > 0.5f )
	{
		p->yr--;
		p->cy++;
	}
	while( p->yr < -0.5f )
	{
		p->yr++;
		p->cy--;
	}

	// check for jump within coyote time
	p->coyote_timer --;
	if( input.btn_jump && p->coyote_timer > 0 )
	{
		jump( p );
	}

	p->jump_buffer--;
	if( input.btn_jump ) p->jump_buffer = jump_buffer_frames;

	// Death
	check_hazards( p );
}





//-----------------------------------------------------------------------------------
// State Wall
//-----------------------------------------------------------------------------------
static void state_player_wall( PlayerController* p )
{
	int dir;

	// Update player input
	update_input_map(&input);

	// update direction
	dir = DIR;
	if( dir == 0 )
	{
		p->wall_dir_timer--;
		if( p->wall_dir_timer <= 0 )
		{
			// falling
			p->coyote_timer = coyote_frames * 0.5;
			p->state_nxt = PLAYERSTATE_AIR;
		}
		else
		{
			p->wall_dir_timer = 6;
		}
	}

	p->wall_timer--;
	if( p->wall_timer <= 0 )
	{
		p->wall_timer--;

		// check if still colliding with wall
		if( dir > 0 )
		{
			if( has_collision( p->cx + 1, p->cy, p->collision_layer, 1 ) && p->xr >= 0.5 )
			{
				p->xr = 0.5;
				p->dx = 0.0;
			}
			else
			{
				p->coyote_timer = coyote_frames * 0.5;
				p->state_nxt = PLAYERSTATE_AIR;
			}
		}
		else
		{
			if( has_collision( p->cx - 1, p->cy, p->collision_layer, 1 ) && p->xr <= 0.5 )
			{
				p->xr = 0.5;
				p->dx = 0.0;
			}
			else
			{
				p->coyote_timer = coyote_frames * 0.5;
				p->state_nxt = PLAYERSTATE_AIR;
			}
		}

		p->dy = term_vely * 0.25;
		p->yr += p->dy;
		if( ( has_collision( p->cx, p->cy + 1, p->collision_layer, 1 ) || has_collision( p->cx, p->cy + 1, p->collision_layer, 2 ) ) && p->yr > 0.5f ) // includes platforms
		{
			p->yr = 0.5f;
			p->dy = 0.0f;
			p->state_nxt = PLAYERSTATE_FLOOR;
		}
		while( p->yr > 0.5f )
		{
			p->yr--;
			p->cy++;
		}
		while( p->yr < -0.5f )
		{
			p->yr++;
			p->cy--;
		}
	}

	if( input.btn_jump )
	{
		jump( p );
		p->dir_nxt = -p->dir_cur;
		p->dx = p->dir_nxt * max_velx * wall_jump_vel_ratio;
		p->wall_timer = wall_jump_margin;
		p->state_nxt = PLAYERSTATE_AIR;
	}

	// Death
	check_hazards( p );
}



//-----------------------------------------------------------------------------------
// State Dead
//-----------------------------------------------------------------------------------
static void state_player_dead( PlayerController* p )
{
	p->dead_timer--;
}





// Utilities
static void jump( PlayerController* p )
{
	p->dy = -jump_vel;
	p->jump_buffer = 0;
	p->coyote_timer = 0;
	p->state_nxt = PLAYERSTATE_AIR;
	p->anim_nxt = 2;

	
}

static int check_hazards( PlayerController* p )
{
	int x = ( int )( (p->cx + p->xr - 0.5 ) * 8 + 0.5 );
	int y = ( int )( ( p->cy + p->yr - 0.5 ) * 8 + 0.5 );
	
	if( has_rect_collision( ( Rectangle ){ x + 2, y + 2, 4, 4 }, p->hazards_layer ) )
	{
		PlaySound( *p->hurt_sfx );
		p->state_nxt = PLAYERSTATE_DEAD;
		p->anim_nxt = 4;
		p->dead_timer  = 60;
		return 1;
	}
	return 0;
}



//-----------------------------------------------------------------------------------
// Additional functions specific to this game
//-----------------------------------------------------------------------------------
int player_check_collision_with_dot( const LDtkEntity_dot* dot_entity, PlayerController* p, int for_damage )
{
	float dx, dy;
	float px, py;
	
	// player position
	px = ( ( float ) p->cx + p->xr ) * 8.0f; 
	py = ( ( float ) p->cy + p->yr ) * 8.0f;

	// check x/y distance to player
	dx = ( float )( dot_entity->px_x + 4 ) - px;
	dy = ( float )( dot_entity->px_y + 4 ) - py;
	if( for_damage == 0 ) // acquiring dot
	{
		if( ( ( dx * dx ) + ( dy * dy ) ) < ( 6 * 6 ) ) // with acquisition, required distance is larger than with damage
			return 1;
	}
	else
	{
		if( ( ( dx * dx ) + ( dy * dy ) ) < ( 5 * 5 ) )
		{
			return 1;
		}
	}
	return 0;
}
