PROG=space_odessy
SRC=main.c 

CC=gcc

LIBS=-lxcb


${PROG}: ${SRC} states
	${CC} -o $@ ${SRC} ${LIBS}
