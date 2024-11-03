

# Output binary name and source files
TARGET = game
SRC = main.c screen_gameplay.c screen_startmenu.c screen_end.c ldtk_tools.c input_map.c player_controller.c aseprite_tools.c

# folder to store assets
ASE_TARGET := ./assets/
# folder to read assets from
ASSETS := ./assets/
# folder to store the header files with data
INCLUDES := ./includes/

# tools folder
TOOLS = ./tools/


# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99 -O3

# Raylib and system dependencies
ifeq ($(OS),Windows_NT)
	RAYLIB = raylib/raylib_win
	PYTHON = python
else
	RAYLIB = raqylib/raylib_macosx_arm
	PYTHON = python3
endif




ifeq ($(OS),Windows_NT)
	LIBS = -lraylib -lm  -lopengl32 -lgdi32 -lwinmm
else
	LIBS = -lraylib -lm -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

# on windows, include this to remove the console
#-Wl,--subsystem,windows



#----------------------------------------------------
# for aseprite
#----------------------------------------------------
ifeq ($(OS),Windows_NT)
	ASEPRITE := C:/Program Files (x86)/Steam/steamapps/common/Aseprite/Aseprite.exe
else
	# this is only for my compiled version of aseprite on Mac OSX
	ASEPRITE := /Users/rluis/Library/Application Support/Steam/steamapps/common/Aseprite/Aseprite.app/Contents/MacOS/aseprite
endif




INCLUDE_DIR = ./$(RAYLIB)/include
LIB_DIR = ./$(RAYLIB)/lib


# Build rules

all: \
	aseprite \
	python \
	sounds \
	src

aseprite: \
	$(ASE_TARGET)tileset.png \
	$(ASE_TARGET)player.png \
	$(ASE_TARGET)dots.png \
	$(ASE_TARGET)background.png \
	$(ASE_TARGET)explosion.png \
	$(ASE_TARGET)title.png \

$(ASE_TARGET)player.png: $(ASSETS)player.aseprite
	"$(ASEPRITE)" -b $(ASSETS)player.aseprite -sheet $(ASE_TARGET)player.png --sheet-columns 4 --data $(ASE_TARGET)player.json --list-tags

$(ASE_TARGET)dots.png: $(ASSETS)dots.aseprite
	"$(ASEPRITE)" -b $(ASSETS)dots.aseprite -sheet $(ASE_TARGET)dots.png --sheet-columns 3 --data $(ASE_TARGET)dots.json --list-tags

$(ASE_TARGET)tileset.png: $(ASSETS)tileset.aseprite
	"$(ASEPRITE)" -b $(ASSETS)tileset.aseprite --save-as $(ASE_TARGET)tileset.png

$(ASE_TARGET)background.png: $(ASSETS)background.aseprite
	"$(ASEPRITE)" -b $(ASSETS)background.aseprite --save-as $(ASE_TARGET)background.png

$(ASE_TARGET)explosion.png: $(ASSETS)explosion.aseprite
	"$(ASEPRITE)" -b $(ASSETS)explosion.aseprite -sheet $(ASE_TARGET)explosion.png --sheet-columns 3 --data $(ASE_TARGET)explosion.json --list-tags

$(ASE_TARGET)title.png: $(ASSETS)title.aseprite
	"$(ASEPRITE)" -b $(ASSETS)title.aseprite --save-as $(ASE_TARGET)title.png


python: \
	tilemaps \
	images \
	animations \


tilemaps: \
	$(INCLUDES)tilemap.h

$(INCLUDES)tilemap.h: $(ASSETS)tilemap.ldtk
	$(PYTHON) $(TOOLS)ldtk2c.py $(ASSETS)tilemap.ldtk > $(INCLUDES)tilemap.h


images: \
	$(INCLUDES)tileset_img.h \
	$(INCLUDES)player_img.h \
	$(INCLUDES)dots_img.h \
	$(INCLUDES)background_img.h \
	$(INCLUDES)explosion_img.h \
	$(INCLUDES)title_img.h \

$(INCLUDES)tileset_img.h: $(ASSETS)tileset.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)tileset.png > $(INCLUDES)tileset_img.h

$(INCLUDES)player_img.h: $(ASSETS)player.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)player.png > $(INCLUDES)player_img.h

$(INCLUDES)dots_img.h: $(ASSETS)dots.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)dots.png > $(INCLUDES)dots_img.h

$(INCLUDES)background_img.h: $(ASSETS)background.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)background.png > $(INCLUDES)background_img.h

$(INCLUDES)explosion_img.h: $(ASSETS)explosion.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)explosion.png > $(INCLUDES)explosion_img.h

$(INCLUDES)title_img.h: $(ASSETS)title.png
	$(PYTHON) $(TOOLS)png2c.py $(ASSETS)title.png > $(INCLUDES)title_img.h

animations: \
	$(INCLUDES)player_anim.h \
	$(INCLUDES)dots_anim.h \
	$(INCLUDES)explosion_anim.h \

$(INCLUDES)player_anim.h: $(ASSETS)player.json
	$(PYTHON) $(TOOLS)asejson2c.py $(ASSETS)player.json > $(INCLUDES)player_anim.h

$(INCLUDES)dots_anim.h: $(ASSETS)dots.json
	$(PYTHON) $(TOOLS)asejson2c.py $(ASSETS)dots.json > $(INCLUDES)dots_anim.h

$(INCLUDES)explosion_anim.h: $(ASSETS)explosion.json
	$(PYTHON) $(TOOLS)asejson2c.py $(ASSETS)explosion.json > $(INCLUDES)explosion_anim.h




sounds: \
	$(INCLUDES)smody_start.h \
	$(INCLUDES)tunnels.h \
	$(INCLUDES)sippypop.h \
	$(INCLUDES)dot.h \
	$(INCLUDES)hurt.h \
	$(INCLUDES)jump.h \
	$(INCLUDES)level.h \


$(INCLUDES)smody_start.h: $(ASSETS)music/smody_start.ogg
	xxd -i $(ASSETS)music/smody_start.ogg $(INCLUDES)smody_start.h

$(INCLUDES)tunnels.h: $(ASSETS)music/tunnels.ogg
	xxd -i $(ASSETS)music/tunnels.ogg $(INCLUDES)tunnels.h

$(INCLUDES)sippypop.h: $(ASSETS)music/sippypop.ogg
	xxd -i $(ASSETS)music/sippypop.ogg $(INCLUDES)sippypop.h

$(INCLUDES)dot.h: $(ASSETS)sfx/dot.wav
	xxd -i $(ASSETS)sfx/dot.wav $(INCLUDES)dot.h

$(INCLUDES)hurt.h: $(ASSETS)sfx/hurt.wav
	xxd -i $(ASSETS)sfx/hurt.wav $(INCLUDES)hurt.h

$(INCLUDES)jump.h: $(ASSETS)sfx/jump.wav
	xxd -i $(ASSETS)sfx/jump.wav $(INCLUDES)jump.h

$(INCLUDES)level.h: $(ASSETS)sfx/level.wav
	xxd -i $(ASSETS)sfx/level.wav $(INCLUDES)level.h

src:
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -L$(LIB_DIR) $(SRC) -o $(TARGET) $(LIBS) -I$(INCLUDES)




WCFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -Os
WINCLUDE_DIR = ./raylib/raylib_web_win/include
WLIB_DIR = ./raylib/raylib_web_win/lib
WFLAGS = -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=67108864 -s FORCE_FILESYSTEM=1


weba:
	emcc -o web/index.html $(SRC) $(WCFLAGS) -I$(WINCLUDE_DIR) -I$(INCLUDES) -L$(WLIB_DIR) $(WLIB_DIR)/libraylib.a $(WFLAGS) -DPLATFORM_WEB -s 'EXPORTED_FUNCTIONS=["_free","_malloc","_main"]' -s EXPORTED_RUNTIME_METHODS=ccall --shell-file shell.html

# Clean the project
clean:
	rm -f $(TARGET)

.PHONY: all clean





