import sys
import os
import PIL
from PIL import Image


data_name = "temp"


def process_png( img ):
	pix = img.load()

	print( "#ifndef %s_H\n#define %s_H\n\n" % ( data_name.upper(), data_name.upper() ) )

	print( "#ifndef TD_PNG2C_DATA\n#define TD_PNG2C_DATA\n" )
	print( "typedef struct\n{\n\tconst int size_x;\n\tconst int size_y;\n\tunsigned char* data;\n} img_data;\n" )
	print( "#endif\n" )

	print( "static img_data img_data_%s = ( img_data )\n{" % data_name )
	# size
	print( "\t.size_x = %d," % ( img.size[0] ) )
	print( "\t.size_y = %d," % ( img.size[1] ) )
	
	print( "\t.data = ( unsigned char[%d] )\n\t{" % ( 4 * img.size[0] * img.size[1] ) )
	count = 0
	print( "\t\t", end = '' )
	for y in range( img.size[1] ):
		for x in range( img.size[0] ):
			r, g, b, a = pix[ x, y];
			print( "0x%02x,0x%02x,0x%02x,0x%02x," % ( r, g, b, a ), end = '' )
			count += 1
			if count >= 5:
				count = 0
				print()
				print( "\t\t", end = '' )
	print()
	print( "\t},")
	print( "};" )

	print( "\n#endif" )


#------------------------------------------------------------
# entry
#------------------------------------------------------------
if __name__ == "__main__":
	# check input arguments
	if len( sys.argv ) != 2:
		print( "Requires a single input argument as the png file." )
		quit()
	png_filename = sys.argv[1]
	png_basename = os.path.basename( png_filename )
	data_name = os.path.splitext( png_basename )[0]

	img = Image.open( png_filename )
	
	if not img:
		quit()
	process_png( img )