PROG=space_odessy
SRC=main.c \
	states/start.c states/play.c states/game_over.c \
	states/objects/shots.c states/objects/asteroids.c

CC=gcc

LIBS=-lxcb -lm


${PROG}: ${SRC} states
	${CC} -o $@ ${SRC} ${LIBS}
