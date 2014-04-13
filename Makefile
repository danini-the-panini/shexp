CC=clang++

LIBS=glew glfw3
LIB_CFLAGS=$(foreach lib,${LIBS},$(shell pkg-config --cflags $(lib)))
LDLIBS=$(foreach lib,${LIBS},$(shell pkg-config --static --libs $(lib)))

WARNING_FLAGS=-Wall -Wextra -Weffc++ -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wunused-parameter -Wstrict-overflow=5 -Wfloat-equal -Wshadow -Wc++0x-compat -Wconversion -Wsign-conversion -Wmissing-declarations -Woverloaded-virtual -Wsign-promo -pedantic
FORMATTING_FLAGS=-fmessage-length=80 -fdiagnostics-show-option
CFLAGS= ${WARNING_FLAGS} ${FORMATTING_FLAGS} ${LIB_CFLAGS} -g -std=c++11 -pipe

LDFLAGS=-g

TARGET=main
OBJECTS=$(addsuffix .o, $(basename $(shell ls *.C | grep -v Test)))
HEADERS=$(addsuffix .h, $(basename $(shell ls *.h | grep -v Test)))

TEST_LDFLAGS=-lboost_unit_test_framework
TEST_LDLIBS=${LDLIBS}
TEST_TARGET=test
TEST_OBJECTS=$(addsuffix .o, $(basename $(shell ls *.C | grep -v main)))

CLEAN_TARGETS=$(addsuffix .o, $(basename $(shell ls *.C))) ${TARGET} ${TARGET}.exe ${TEST_TARGET} *.rpo *.gch makefile.dep

PROJECT=shexp

all: ${OBJECTS}
	${CC} $^ ${LDFLAGS} -o ${TARGET} ${LDLIBS}

test: ${TEST_OBJECTS}
	${CC} ${TEST_LDFLAGS} ${TEST_LDLIBS} $^ -o ${TEST_TARGET}

%.o:
	${CC} ${CFLAGS} -c ${LDFLAGS} $< -o $@

# We override Test.o's compilation so it doesn't use the CFLAGS, this is because
# boost uses coding practices that would cause problems with them.
Test.o: Test.C Makefile
	${CC} -c $< -o $@

clean:
	rm -rf ${CLEAN_TARGETS}

tar:
	make clean; tar -cvjf ${PROJECT}.tar.bz2 *

# Calculate dependencies based on output from gcc -MM.
makefile.dep: *.[Ch]
	for i in *.C; do gcc -MM "$${i}"; done > $@
include makefile.dep
