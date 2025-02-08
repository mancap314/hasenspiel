CC = gcc
EMCC=emcc
ifeq ($(OS),Windows_NT)
	EXT = exe
	RM = del
else 
	EXT = o
	RM = rm
endif

CFLAGS = -O3
RAYFLAGS = -lraylib -lm

hasen_test: hasen_test.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	./$@.$(EXT)

examples: examples.c hasen_play.c hasen_utils.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	# ./$@.$(EXT)

hasenray: all_estates.c hasen_play.c hasenray.c hasen_utils.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS) $(RAYFLAGS)
	./$@.$(EXT)

hasenrayweb: all_estates.c hasenray.c hasen_play.c hasen_utils.c hasen.c
	$(EMCC) -o web/$@.html $^ -D_DEFAULT_SOURCE \
		-I. -I ${HOME}/raylib/src -I ${HOME}/raylib/src/external -I ${HOME}/raygui/src \
		-L. -L ${HOME}/raylib/src -L ${HOME}/raygui/src \
		-s USE_GLFW=3 -s ASYNCIFY -s TOTAL_STACK=16MB -s TOTAL_MEMORY=32MB -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 \
		--shell-file ${HOME}/raylib/src/shell.html ${HOME}/raylib/src/web/libraylib.web.a \
		--preload-file resources \
		-DPLATFORM_WEB 


.PHONY: clean

clean:
	$(RM) *.$(EXT)
