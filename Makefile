PROG=space_odessy

STATES=states/start.c states/play.c states/game_over.c
OBJECTS=states/objects/shots.c states/objects/asteroids.c
SRC=main.c $(STATES) $(OBJECTS)

CFLAGS=-Wall -Wextra  

CC=gcc

LIBS=-lxcb -lm -lxcb-image -lxcb-shm -lfreetype

${PROG}: ${SRC} states
	${CC} -o $@ ${SRC} ${LIBS} ${CFLAGS}
