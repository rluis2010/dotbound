#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include "ldtk_tools.h"
#include "tilemap.h"



void draw_ldtk_layer( const LDtkLayer* layer, Texture2D* tileset_texture )
{
	int i;
	Rectangle source_rect;
	Rectangle dest_rect;
	source_rect = ( Rectangle ){ 0, 0, layer->gridSize, layer->gridSize };
	dest_rect = ( Rectangle ){ 0, 0, layer->gridSize, layer->gridSize };

	if( layer->type == LDTK_LAYERTYPE_ENTITIES ) return;
	for( i = 0; i < layer->ntiles; i++ )
	{
		source_rect.x = layer->tiles[i].src_x;
		source_rect.y = layer->tiles[i].src_y;
		dest_rect.x = layer->tiles[i].px_x;
		dest_rect.y = layer->tiles[i].px_y;
		DrawTexturePro( *tileset_texture, source_rect, dest_rect, ( Vector2 ){ 0, 0 }, 0.0f, WHITE );
	}
}

void draw_ldtk_level( const LDtkLevel* level, Texture2D* tileset_texture )
{
	int i;
	for( i = level->nlayers - 1; i >= 0; i-- )
		draw_ldtk_layer( &level->layers[i], tileset_texture );
}

int has_collision( int cx, int cy, const LDtkLayer* layer, int gridval )
{
	int cpos;
	if( cx < 0 || cy < 0 || cx >= layer->tileWid || cy >= layer->tileHei ) return 0;
	if( layer->type == LDTK_LAYERTYPE_INTGRID )
	{
		cpos = cx + cy * layer->tileWid;
		if( layer->nintGridCsv > cpos && layer->intGridCsv[cpos] == gridval ) return 1;
	}
	return 0;
}

int has_rect_collision( Rectangle p, const LDtkLayer* layer )
{
	//bool CheckCollisionRecs(Rectangle rec1, Rectangle rec2);
	int i;

	for( i = 0; i < layer->ntiles; i++ )
	{
		// check distance to tile
		if( abs( layer->tiles[i].px_x - ( int )p.x ) > 2 * layer->gridSize || abs( layer->tiles[i].px_y - ( int )p.y ) > 2 * layer->gridSize ) continue;
		// rectangle collision
		if( CheckCollisionRecs( p, ( Rectangle ){ layer->tiles[i].px_x, layer->tiles[i].px_y, layer->gridSize, layer->gridSize } ) ) return 1;
	}
	return 0;
}


const LDtkLayer* get_layer_by_name( char* name, const LDtkLevel* level )
{
	int i;
	for( i = 0; i < level->nlayers; i++ )
		if( strcmp( name, level->layers[i].identifier ) == 0 ) return &level->layers[i];
	return NULL;
}