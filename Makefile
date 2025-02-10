CC = gcc
EMCC=emcc
ifeq ($(OS),Windows_NT)
	EXT = exe
	RM = del
	BASE_PATH = C:\Users\manue\Documents\code
	RAYFLAGS = -I"$(BASE_PATH)\raylib\src" -I"$(BASE_PATH)\raygui\src" "$(BASE_PATH)\raylib\src\libraylib.a" -lopengl32 -lgdi32 -lwinmm
else 
	EXT = o
	RM = rm
	BASE_PATH = ${HOME}
	RAYFLAGS = -lraylib -lm
endif

CFLAGS = -O3

hasen_test: hasen_test.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	./$@.$(EXT)

generate_all_estates: examples.c hasen_utils.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	# ./$@.$(EXT)

hasenray: all_estates.c hasen_play.c hasenray.c hasen_utils.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS) $(RAYFLAGS)
	./$@.$(EXT)

hasenrayweb: all_estates.c hasenray.c hasen_play.c hasen_utils.c hasen.c
	$(EMCC) -o web/$@.html $^ -D_DEFAULT_SOURCE \
		-I. -I $(BASE_PATH)/raylib/src -I $(BASE_PATH)/raylib/src/external -I $(BASE_PATH)/raygui/src \
		-L. -L $(BASE_PATH)/raylib/src -L $(BASE_PATH)/raygui/src \
		-s USE_GLFW=3 -s ASYNCIFY -s TOTAL_STACK=16MB -s TOTAL_MEMORY=32MB -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 \
		--shell-file $(BASE_PATH)/raylib/src/shell.html $(BASE_PATH)/raylib/src/web/libraylib.web.a \
		--preload-file resources \
		-DPLATFORM_WEB \
		-Os


.PHONY: clean

clean:
	$(RM) *.$(EXT)
