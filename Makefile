CC = gcc
ifeq ($(OS),Windows_NT)
	EXT = exe
	RM = del
else 
	EXT = o
	RM = rm
endif

CFLAGS = -O3

hasen_test: hasen_test.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	./$@.$(EXT)

examples: examples.c hasen_play.c hasen_utils.c hasen.c
	$(CC) -o $@.$(EXT) $^ $(CFLAGS)
	# ./$@.$(EXT)

.PHONY: clean

clean:
	$(RM) *.$(EXT)
