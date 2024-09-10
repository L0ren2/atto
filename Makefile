
.PHONY: all clean

OBJ=atto.o

CXX=/usr/bin/g++
CXXFLAGS=-Wall -Wextra -Wpedantic -pedantic-errors -g -Og -std=c++11

all: bin/atto
	@#

bin/atto: obj/$(OBJ)
	@mkdir -p bin
	$(CXX) -o $@ $< -lncursesw

obj/%.o: %.cpp
	@mkdir -p obj
	$(CXX) $(CXXFLAGS) -o $@ -c $^

clean:
	rm -r bin obj
