LEXER_TOOL := flex
CXX := g++
CPP_SRCS := $(wildcard *.cpp)
OBJ_SRCS := parser.o lexer.o $(CPP_SRCS:.cpp=.o)
DEPS := $(OBJ_SRCS:.o=.d)
FLAGS=-pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wundef -Werror -Wno-unused


TESTFILES := $(wildcard tests/*.cRAP)
TESTS := $(TESTFILES:.cRAP=)

.PHONY: all clean test cleantest

all:
	make cRAPc

clean:
	rm -rf *.output *.o *.cc *.hh $(DEPS) cRAPc

-include $(DEPS)

cRAPc: $(OBJ_SRCS)
	$(CXX) $(FLAGS) -g -std=c++14 -o $@ $(OBJ_SRCS)

%.o: %.cpp
	$(CXX) $(FLAGS) -g -std=c++14 -MMD -MP -c -o $@ $<

parser.o: parser.cc
	$(CXX) $(FLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-switch-default -g -std=c++14 -MMD -MP -c -o $@ $<

parser.cc: cRAP.yy
	bison --defines=grammar.hh -v $<

lexer.yy.cc: cRAP.l
	$(LEXER_TOOL) --outfile=lexer.yy.cc $<

lexer.o: lexer.yy.cc
	$(CXX) $(FLAGS) -Wno-sign-compare -Wno-sign-conversion -Wno-old-style-cast -Wno-switch-default -g -std=c++14 -c lexer.yy.cc -o lexer.o

test: all
	$(MAKE) -C p4_tests/
cleantest:
	$(MAKE) -C p4_tests/ clean
	
