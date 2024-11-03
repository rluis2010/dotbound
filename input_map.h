#ifndef INPUT_MAP_H
#define INPUT_MAP_H

typedef struct
{
	int btn_left;
	int btn_right;
	int btn_jump;
} InputMap;

void update_input_map( InputMap* input_map );

#endif