PROG=space_odessy

UTIL=src/util/image.c src/util/sound.c src/util/font.c src/util/window.c
STATES=src/states/start.c src/states/play.c src/states/game_over.c
OBJECTS=src/objects/shots.c src/objects/asteroids.c

SRC=src/main.c $(STATES) $(OBJECTS) $(UTIL)
CC=gcc
LIBS=-lxcb -lm -lxcb-image -lxcb-shm -lasound 

WIN_SRC=src/win.c $(UTIL) $(STATES) $(OBJECTS) 
WIN_CC=x86_64-w64-mingw32-gcc
WIN_LIBS=-mwindows -lgdi32 -lm -lwinmm

${PROG}: ${SRC}
	${CC} -o $@ ${SRC} ${LIBS} 

win:
	${WIN_CC} $(WIN_SRC) -o win ${WIN_LIBS} -static 

