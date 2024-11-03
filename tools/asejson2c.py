import sys
import os
import json

data_name = "temp"


def process_asejson( data ):

	print( "#ifndef %s_JSON_H\n#define %s_JSON_H\n\n" % ( data_name.upper(), data_name.upper() ) )


	# print( "#ifndef TD_ASEJSON2C_DATA\n#define TD_ASEJSON2C_DATA\n" )

	# print( "typedef struct\n{" )
	# print( "\tint x; int y;" )
	# print( "\tint w; int h;" )
	# print( "\tint duration_ms;" )
	# print( "} ASEpriteFrame;\n" )

	# print( "typedef struct\n{" )
	# print( "\tchar* name;")
	# print( "\tint from; int to;" )
	# print( "\tint dir;" )
	# print( "\tASEpriteFrame* frames;")
	# print( "} ASEpriteAnim;\n" )

	# print( "#endif\n\n" )
	print( "\n#include \"../aseprite_tools.h\"\n")

	print( "static const ASEpriteFrame frames_%s[] =\n{" % ( data_name ) )
	for key, framedata in data["frames"].items():
		print( "\t{ .x = %d, .y = %d, .w = %d, .h = %d, .duration_ms = %d }," % ( framedata["frame"]["x"], framedata["frame"]["y" ], framedata["frame"]["w"], framedata["frame"]["h" ], framedata["duration"] ) )
	print( "};\n" )

	print( "static ASEpriteAnim animations_%s[] =\n{" % ( data_name ) )
	for anim in data["meta"]["frameTags"]:
		#print( anim )
		print( "\t{\n\t\t.name = \"%s\"," % ( anim["name"] ) )
		print( "\t\t.from = %d, .to = %d, .dir = %d," % ( anim["from" ], anim["to"], 1 if anim["direction"] == "forward" else -1 ) )
		if "repeat" in anim:
			print( "\t\t.rep = 0," )
		else:
			print( "\t\t.rep = 1," )
		print( "\t\t.nframes = %d," % ( anim["to"] - anim["from"] + 1 ) )
		print( "\t\t.frames = ( ASEpriteFrame[] )\n\t\t{ ", end='' )
		for idx in range( anim["from" ], anim["to"] + 1 ):
			print( "frames_%s[%d], " % ( data_name, idx ), end='' )
		print( "},")
		print( "\t},")
	print( "};" )
		

# "frameTags": [
#    { "name": "idle", "from": 0, "to": 0, "direction": "forward", "color": "#000000ff" },
#    { "name": "run", "from": 1, "to": 10, "direction": "forward", "color": "#000000ff" },
#    { "name": "stop_run", "from": 11, "to": 13, "direction": "forward", "color": "#000000ff" },
#    { "name": "up", "from": 14, "to": 19, "direction": "forward", "color": "#000000ff" },
#    { "name": "down", "from": 20, "to": 25, "direction": "forward", "color": "#000000ff" },
#    { "name": "land", "from": 26, "to": 29, "direction": "forward", "color": "#000000ff" }
#   ]






	# print( "static png_data png_data_%s = ( png_data )\n{" % data_name )
	# # size
	# print( "\t.size_x = %d," % ( img.size[0] ) )
	# print( "\t.size_y = %d," % ( img.size[1] ) )
	
	# print( "\t.data = ( unsigned char[%d] )\n\t{" % ( 4 * img.size[0] * img.size[1] ) )
	# count = 0
	# print( "\t\t", end = '' )
	# for y in range( img.size[1] ):
	# 	for x in range( img.size[0] ):
	# 		r, g, b, a = pix[ x, y];
	# 		print( "0x%02x,0x%02x,0x%02x,0x%02x," % ( r, g, b, a ), end = '' )
	# 		count += 1
	# 		if count >= 5:
	# 			count = 0
	# 			print()
	# 			print( "\t\t", end = '' )
	# print()
	# print( "\t},")
	# print( "};" )

	print( "\n#endif" )


#------------------------------------------------------------
# entry
#------------------------------------------------------------
if __name__ == "__main__":
	# check input arguments
	if len( sys.argv ) != 2:
		print( "Requires a single input argument as the json file." )
		quit()
	json_filename = sys.argv[1]

	json_basename = os.path.basename( json_filename )
	data_name = os.path.splitext( json_basename )[0]

	#if verbose > 0: print( "Processing file: " + ldtk_filename )
	with open( json_filename, "r" ) as file:
		rawdata = json.load( file )
	process_asejson( rawdata )