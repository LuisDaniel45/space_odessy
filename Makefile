PROG=space_odessy

STATES=src/states/start.c src/states/play.c src/states/game_over.c
OBJECTS=src/objects/shots.c src/objects/asteroids.c
SRC=src/main.c $(STATES) $(OBJECTS)

CFLAGS=-Wall -Wextra  

CC=gcc

LIBS=-lxcb -lm -lxcb-image -lxcb-shm -lfreetype -lopenal -lalut

${PROG}: src 
	${CC} -o $@ ${SRC} ${LIBS} ${CFLAGS}
