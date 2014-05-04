CC=clang++

LIBS=glew glfw3
LIB_CFLAGS=$(foreach lib,${LIBS},$(shell pkg-config --cflags $(lib)))
LDLIBS=$(foreach lib,${LIBS},$(shell pkg-config --static --libs $(lib)))

WARNING_FLAGS=-Wall -Wextra -Weffc++ -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -Wstrict-overflow=5 -Wfloat-equal -Wshadow -Wc++0x-compat -Wconversion -Wsign-conversion -Wmissing-declarations -Woverloaded-virtual -Wsign-promo -pedantic
FORMATTING_FLAGS=-fmessage-length=80 -fdiagnostics-show-option
CFLAGS= ${WARNING_FLAGS} ${FORMATTING_FLAGS} ${LIB_CFLAGS} -g -std=c++11 -pipe

LDFLAGS=-g

TARGET=main
OBJECTS=$(addsuffix .o, $(basename $(shell ls *.C)))
HEADERS=$(addsuffix .h, $(basename $(shell ls *.h)))

CLEAN_TARGETS=$(addsuffix .o, $(basename $(shell ls *.C))) ${TARGET} ${TARGET}.exe *.rpo *.gch makefile.dep

CODE_GEN_DIR=sh_code_gen
CODE_GEN_TARGET=sh_code_gen

SH_BANDS=3

CODE_GENERATOR=gen_sh_functions.rb

all: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o ${TARGET} ${LDLIBS}

${OBJECTS}:
	${CC} ${CFLAGS} -c ${LDFLAGS} $< -o $@

clean:
	$(MAKE) -C ${CODE_GEN_DIR} clean
	rm -rf ${CLEAN_TARGETS}

# Calculate dependencies based on output from gcc -MM.
makefile.dep: sh_functions.h *.[Ch]
	for i in *.C; do gcc ${CFLAGS} -MM "$${i}"; done > $@

sh_functions.h: ${CODE_GEN_DIR}/${CODE_GEN_TARGET} ${CODE_GEN_DIR}/${CODE_GENERATOR}
	$< ${SH_BANDS} | ruby ${CODE_GEN_DIR}/${CODE_GENERATOR} ${SH_BANDS} > $@

${CODE_GEN_DIR}/${CODE_GEN_TARGET}: ${CODE_GEN_DIR}/*.[Ch]
	$(MAKE) -C ${CODE_GEN_DIR}

include makefile.dep
