CPP=g++
CFLAGS=
SOURCE=main.cpp
OBJ=$(SOURCE:.cpp=.o)

COMMON_SOURCE=lexer.cpp Parsing.cpp quadruple.cpp symbolTable.cpp
COMMON_OBJECT=$(COMMON_SOURCE:.cpp=.o)

EXE=toycc

.PHONY : all clean

all : $(EXE)

$(EXE) : % : main.o $(COMMON_OBJECT)
	$(CPP) $(CFLAGS) $^ -o $@

$(COMMON_OBJECT) : %.o : %.cpp
	$(CPP) -c $(CFLAGS) $^ -o $@

$(OBJ) : %.o : %.cpp
	$(CPP) -c $(CFLAGS) $^ -o $@

clean :
	-rm -rf $(EXE)
	-rm -rf $(OBJ) $(COMMON_OBJECT)