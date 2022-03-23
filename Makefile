#
#  Main authors:
#     Stefano Gualandi <stefano.gualandi@gmail.com>
#
#  Copyright:
#     Stefano Gualandi, 2011
#

include config.mk

rlf: ${LIB}/read_dimacs_bin.o ${SRC}/rlf.cpp
	${COMPILER} -o bin/rlf src/rlf.cpp -I${INCLUDE} ${LIB}/read_dimacs_bin.o

rlfPlus: ${LIB}/read_dimacs_bin.o ${SRC}/rlfPlus.cpp
	${COMPILER} -o ${BIN}/rlfPlus ${SRC}/rlfPlus.cpp -I${INCLUDE} ${LIB}/read_dimacs_bin.o

lazyRlf: ${LIB}/read_dimacs_bin.o ${SRC}/lazyRlf.cpp
	${COMPILER} -o ${BIN}/lazyRlf ${SRC}/lazyRlf.cpp -I${INCLUDE} ${LIB}/read_dimacs_bin.o

rlfAdaptive: ${LIB}/read_dimacs_bin.o ${SRC}/rlfAdaptive.cpp
	${COMPILER} -o ${BIN}/rlfAdaptive ${SRC}/rlfAdaptive.cpp -I${INCLUDE} ${LIB}/read_dimacs_bin.o

# Testing utilities
generator: ${SRC}/generator.cpp
	${COMPILER} -DNDEBUG -o ${BIN}/generator ${SRC}/generator.cpp -I${INCLUDE} -I${BOOST_INCLUDE} 

converter: ${SRC}/converter.cpp
	${COMPILER} -DNDEBUG -o ${BIN}/converter ${SRC}/converter.cpp -I${INCLUDE} -I${BOOST_INCLUDE}

# My Libs
${LIB}/read_dimacs_bin.o: ${SRC}/read_dimacs_bin.cpp
	${COMPILER} -o ${LIB}/read_dimacs_bin.o -c ${SRC}/read_dimacs_bin.cpp -I${INCLUDE}


# Clean the repositories
clean:
	rm -f ${LIB}/*
	rm -f ${BIN}/*
	rm -f *~ ${SRC}/*~ ${INCLUDE}/*~
