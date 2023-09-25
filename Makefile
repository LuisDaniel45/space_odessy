PROG=space_odessy

UTIL=src/util/image.c src/util/sound.c src/util/font.c src/util/window.c
STATES=src/states/start.c src/states/play.c src/states/game_over.c
OBJECTS=src/objects/shots.c src/objects/asteroids.c

SRC=src/main.c $(STATES) $(OBJECTS) $(UTIL)

CFLAGS=-Wall -Wextra  

CC=gcc

LIBS=-lxcb -lm -lxcb-image -lxcb-shm -lfreetype -lopenal -lalut

${PROG}: src 
	${CC} -o $@ ${SRC} ${LIBS} ${CFLAGS}
