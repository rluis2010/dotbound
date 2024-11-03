#ifndef LDTK_TOOLS_H
#define LDTK_TOOLS_H

//#include <raylib.h>
#include "tilemap.h"

// function definitions
void draw_ldtk_level( const LDtkLevel* level, Texture2D* tileset_texture );
void draw_ldtk_layer( const LDtkLayer* layer, Texture2D* tileset_texture );
int has_collision( int cx, int cy, const LDtkLayer* layer, int gridval );
int has_rect_collision( Rectangle p, const LDtkLayer* layer );
const LDtkLayer* get_layer_by_name( char* name, const LDtkLevel* level );

#endif