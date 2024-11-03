"""
LDTk2C converts an LDTk tilemap file to a C header file for use in game development
The C header file has the necessary information to draw the tilemap and entities.
"""

import sys
import os
import json

#------------------------------------------------------------
# Parameters
#------------------------------------------------------------
ldtk_filename = ""
ldtk_entity_types = []

#------------------------------------------------------------
# Useful methods
#------------------------------------------------------------
def print_json( d ):
	print( json.dumps( d, sort_keys=True, indent=4 ) )


def get_entity_type_name( basename ):
	return "LDtkEntity_" + basename

def get_entity_variable_name( basename ):
	return basename + "s"

#------------------------------------------------------------
# start processing
#------------------------------------------------------------
def process_ldtk( data ):
	global ldtk_entity_types
	# base
	print( "#ifndef LDTK2C_H\n#define LDTK2C_H\n\n")

	#------------------------------------------------------------
	# constants
	#------------------------------------------------------------
	# Types of layer
	print( "#define LDTK_LAYERTYPE_TILES 0")
	print( "#define LDTK_LAYERTYPE_INTGRID 1")
	print( "#define LDTK_LAYERTYPE_AUTOLAYER 2")
	print( "#define LDTK_LAYERTYPE_ENTITIES 3")
	print( "\n" )

	
	#------------------------------------------------------------
	# print structure definitions
	#------------------------------------------------------------
	# LDtk types of entities
	print( "//---------------------------------------------" )
	print( "// Entity definitions" )
	print( "//---------------------------------------------" )
	ldtk_entity_types = []
	for entity_type in data["defs"]["entities"]:
		ldtk_entity_types.append( entity_type["identifier"] )
		print( "typedef struct\n{" )
		print( "\tconst char* identifier;" )
		print( "\tconst char* iid;" )
		print( "\tconst int grid_x;" )
		print( "\tconst int grid_y;" )
		print( "\tconst int width;" )
		print( "\tconst int height;" )
		print( "\tconst int px_x;" )
		print( "\tconst int px_y;" )
		print( "\tconst int world_x;" )
		print( "\tconst int world_y;" )
		for fieldDef in entity_type["fieldDefs"]:
			match fieldDef["__type"]:
				case "Int":
					print( "\tconst int " + fieldDef["identifier"] + ";" )
				case "Float":
					print( "\tconst float " + fieldDef["identifier"] + ";" )
				case "Bool":
					print( "\tconst int " + fieldDef["identifier"] + ";" )
				case "String":
					print( "\tconst char* " + fieldDef["identifier"] + ";" )
		print( "} " + get_entity_type_name( entity_type["identifier"] ) + ";\n" )
	
	print( "//---------------------------------------------" )
	print( "// Common definitions" )
	print( "//---------------------------------------------" )
	# LDtkTile
	print( "typedef struct\n{" )
	print( "\tconst int px_x;" )
	print( "\tconst int px_y;" )
	print( "\tconst int src_x;" )
	print( "\tconst int src_y;" )
	print( "\tconst int f;" )
	print( "\tconst int t;" )
	print( "\tconst int a;" )
	print( "\tconst int d_x;" )
	print( "\tconst int d_y;" )
	print( "} LDtkTile;\n" )


	


	# LDtkLayer
	print( "typedef struct\n{" )
	print( "\tconst char* identifier;" )
	print( "\tconst char* iid;" )
	print( "\tconst int type;" )
	print( "\tconst int gridSize;" )
	print( "\tconst int tileWid;" )
	print( "\tconst int tileHei;" )
	print( "\tconst int ntiles;" )
	print( "\tconst LDtkTile* tiles;" )
	print( "\tconst int nintGridCsv;" )
	print( "\tconst int* intGridCsv;" )
	for entity_type in data["defs"]["entities"]:
		entity_id = entity_type["identifier"]
		print( "\tconst int n" + get_entity_variable_name( entity_id ) + ";" )
		print( "\tconst " + get_entity_type_name( entity_id ) + "* " + get_entity_variable_name( entity_id ) + ";" )
	print( "} LDtkLayer;\n" )
	
	# LDtkLevel
	print( "typedef struct\n{" )
	print( "\tconst char* identifier;" )
	print( "\tconst char* iid;" )
	print( "\tconst int pxWid;" )
	print( "\tconst int pxHei;" )
	print( "\tconst int worldX;" )
	print( "\tconst int worldY;" )
	print( "\tconst int nlevels;" )
	print( "\tconst int is_final;" )
	print( "\tconst int nlayers;")
	print( "\tconst LDtkLayer* layers;" )
	print( "} LDtkLevel;\n" )


	print( "//---------------------------------------------" )
	print( "// Level data" )
	print( "//---------------------------------------------" )
	# Output Levels
	print( "static const LDtkLevel LDtkLevels[] =\n{")
	rawlevels_data = data["levels"]
	for rawlevel_data in rawlevels_data:
		curlevel_rawdata = rawlevel_data
		external_relpath = curlevel_rawdata["externalRelPath"]
		if external_relpath:
			# the level is in another file
			basepath = os.path.dirname( ldtk_filename )
			full_filename = os.path.join( basepath, external_relpath )
			#if verbose > 0: print( "loading level file: " + full_filename )
			with open( full_filename, "r" ) as file:
				
				curlevel_rawdata = json.load( file )
		process_level( curlevel_rawdata, len( rawlevels_data ), rawlevel_data == rawlevels_data[-1] )
	print( "};\n" )


	# end of the header file
	print( "#endif" )


#------------------------------------------------------------
# process one level
#------------------------------------------------------------
def process_level( data, nlevels, islast ):
	# gather layer information
	rawlayers_data = data["layerInstances"]
	#
	print( "\t{" )
	# basic information
	print( "\t\t.identifier = \"" + data["identifier"] + "\"," )
	print( "\t\t.iid = \"" + data["iid"] + "\"," )
	print( "\t\t.pxWid = " + str( data["pxWid"] ) + "," )
	print( "\t\t.pxHei = " + str( data["pxHei"] ) + "," )
	print( "\t\t.worldX = " + str( data["worldX"] ) + "," )
	print( "\t\t.worldY = " + str( data["worldY"] ) + "," )
	print( "\t\t.nlevels = " + str( nlevels ) + "," )
	print( "\t\t.is_final = " + ( "1" if islast else "0" ) + "," )
	# layers
	print( "\t\t.nlayers = " + str( len( rawlayers_data ) ) + "," )
	print( "\t\t.layers = ( LDtkLayer[] )\n\t\t{" )
	for rawlayer_data in rawlayers_data:
		process_layer( rawlayer_data, data )
	# end of the layers
	print( "\t\t}," )
	# end of the level
	print( "\t}," )
	


def process_layer( data, level ):
	layer_type = "LDTK_LAYERTYPE_TILES"
	tiles = data["gridTiles"]
	ntiles = len( tiles )
	ngrid = 0
	# nentities = 0
	if data["__type"] == "AutoLayer":
		layer_type = "LDTK_LAYERTYPE_AUTOLAYER"
		tiles = data["autoLayerTiles"]
		ntiles = len( tiles )
	if data["__type"] == "IntGrid":
		layer_type = "LDTK_LAYERTYPE_INTGRID"
		tiles = data["autoLayerTiles"]
		ntiles = len( tiles )
		ngrid = len( data["intGridCsv"] )
		nx = 16
	if data["__type"] == "Entities":
		layer_type = "LDTK_LAYERTYPE_ENTITIES"
		tiles = {}
		ntiles = 0
		# nentities = len( data["entityInstances"] )
	
	print( "\t\t\t{" )
	print( "\t\t\t\t.identifier = \"" + data["__identifier"] + "\"," )
	print( "\t\t\t\t.iid = \"" + data["iid"] + "\"," )
	print( "\t\t\t\t.type = " + layer_type + "," )
	print( "\t\t\t\t.gridSize = " + str( data["__gridSize"] ) + "," )
	print( "\t\t\t\t.tileWid = " + str( level["pxWid"] / data["__gridSize"] ) + "," )
	print( "\t\t\t\t.tileHei = " + str( level["pxHei"] / data["__gridSize"] ) + "," )
	
	# if ntiles == 0:
	# 	print( "\t\t\t\t.tiles = ( LDtkTile[] ){}," )
		
	if ntiles > 0:
		print( "\t\t\t\t.ntiles = " + str( ntiles ) + "," )
		print( "\t\t\t\t.tiles = ( LDtkTile[] )\n\t\t\t\t{")
		for tile in tiles:
			print( "\t\t\t\t\t{ ", end = '' )
			print( ".px_x = " + str( tile["px"][0] ) + ", ", end = '' )
			print( ".px_y = " + str( tile["px"][1] ) + ", ", end = '' )
			print( ".src_x = " + str( tile["src"][0] ) + ", ", end = '' )
			print( ".src_y = " + str( tile["src"][1] ) + ", ", end = '' )
			print( ".f = " + str( tile["f"] ) + ", ", end = '' )
			print( ".t = " + str( tile["t"] ) + ", ", end = '' )
			print( ".a = " + str( tile["a"] ) + ", ", end = '' )
			print( ".d_x = " + str( tile["d"][0] ), end = '' )
			if len( tile["d"] ) > 1:
				print( ", .d_y = " + str( tile["d"][1] ), end = '' )
			print( " }," )
		print( "\t\t\t\t},")

		if ngrid > 0:
			print( "\t\t\t\t.nintGridCsv = " + str( ngrid ) + "," )
			print( "\t\t\t\t.intGridCsv = ( int[] )\n\t\t\t\t{")
			count = 0
			for g in data["intGridCsv"]:
				if count == 0: print( "\t\t\t\t\t", end = '' )
				print( str( g ) + ", ", end = '' )
				count += 1
				if count >= nx:
					count = 0
					print()
			print( "\t\t\t\t}," )
	
	for entity_type in ldtk_entity_types:
		entities_of_this_type = []
		for entity in data["entityInstances"]:
			if entity["__identifier"] == entity_type:
				entities_of_this_type.append( entity )
		if len( entities_of_this_type ) == 0: continue
		print( "\t\t\t\t.n" + get_entity_variable_name( entity_type ) + " = " + str( len( entities_of_this_type ) ) + "," )
		print( "\t\t\t\t." + get_entity_variable_name( entity_type ) + " = ( " + get_entity_type_name( entity_type ) + "[] )" )
		print( "\t\t\t\t{" )
		for entity in entities_of_this_type:
			process_entity( entity )
		print( "\t\t\t\t},")
	print( "\t\t\t}," )
	

def process_entity( entity ):
	print( "\t\t\t\t\t{")
	print( "\t\t\t\t\t\t.identifier = \"" + entity["__identifier"] + "\"," )
	print( "\t\t\t\t\t\t.iid = \"" + entity["iid"] + "\"," )
	print( "\t\t\t\t\t\t.grid_x = " + str( entity["__grid"][0] ) + ", .grid_y = " + str( entity["__grid"][1] ) + "," )
	print( "\t\t\t\t\t\t.width = " + str( entity["width"] ) + ", .height = " + str( entity["height"] ) + "," )
	print( "\t\t\t\t\t\t.px_x = " + str( entity["px"][0] ) + ", .px_y = " + str( entity["px"][1] ) + "," )
	print( "\t\t\t\t\t\t.world_x = " + str( entity["__worldX"] ) + ", .world_y = " + str( entity["__worldY"] ) + "," )
	
	for fieldInstance in entity["fieldInstances"]:
		field_id = fieldInstance["__identifier"]
		field_val = fieldInstance["__value"]
		field_type = fieldInstance["__type"]
		if field_type == "Int" or field_type == "Float":
			print( "\t\t\t\t\t\t." + field_id + " = " + str( field_val ) + "," )
		if field_type == "Bool":
			print( "\t\t\t\t\t\t." + field_id + " = " + str( 1 if field_val else 0 ) + "," )
		if field_type == "String":
			print( "\t\t\t\t\t\t." + field_id + " = \"" + field_val + "\"," )
	print( "\t\t\t\t\t}," )

# def process_field_instance( data ):
# 	# print( data )
# 	print( ".fieldInstances = ( void[] ){ 0, 1, 2 },")
# 	# match data["__type"]:
# 	# 	case "Int":
# 	pass





#------------------------------------------------------------
# entry
#------------------------------------------------------------
if __name__ == "__main__":
	# check input arguments
	if len( sys.argv ) != 2:
		print( "Requires a single input argument as the ldtk file." )
		quit()
	ldtk_filename = sys.argv[1]
	#if verbose > 0: print( "Processing file: " + ldtk_filename )
	with open( ldtk_filename, "r" ) as file:
		rawdata = json.load( file )
	process_ldtk( rawdata )
	#print_json( rawdata )











