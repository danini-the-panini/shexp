CC=clang++

LIBS=
LIB_CFLAGS=$(foreach lib,${LIBS},$(shell pkg-config --cflags $(lib)))
LDLIBS=$(foreach lib,${LIBS},$(shell pkg-config --static --libs $(lib)))

WARNING_FLAGS=-Wall -Wextra -Weffc++ -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -Wstrict-overflow=5 -Wfloat-equal -Wshadow -Wc++0x-compat -Wconversion -Wsign-conversion -Wmissing-declarations -Woverloaded-virtual -Wsign-promo -pedantic
FORMATTING_FLAGS=-fmessage-length=80 -fdiagnostics-show-option
CFLAGS= ${WARNING_FLAGS} ${FORMATTING_FLAGS} ${LIB_CFLAGS} -g -std=c++11 -pipe

LDFLAGS=-g

TARGET=sh_code_gen
OBJECTS=$(addsuffix .o, $(basename $(shell ls *.C)))
HEADERS=$(addsuffix .h, $(basename $(shell ls *.h)))

CODE_OUTPUT=sh_functions.h

CLEAN_TARGETS=$(addsuffix .o, $(basename $(shell ls *.C))) ${TARGET} ${TARGET}.exe *.rpo *.gch makefile.dep ${CODE_OUTPUT}

all: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o ${TARGET} ${LDLIBS}

%.o:
	${CC} ${CFLAGS} -c ${LDFLAGS} $< -o $@

clean:
	rm -rf ${CLEAN_TARGETS}

# Calculate dependencies based on output from gcc -MM.
makefile.dep: *.[Ch]
	for i in *.C; do gcc ${CFLAGS} -MM "$${i}"; done > $@
include makefile.dep
