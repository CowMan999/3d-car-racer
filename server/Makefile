CXX= g++
CC= gcc
CXXFLAGS= -std=c++20 -Wall -Wextra -Werror -s -DSFML_STATIC -Wpedantic -DTAGLIB_STATIC -O3 -D_IRRIMGUI_STATIC_LIB_ -D_IRR_STATIC_LIB_
CFLAGS= -std=c99 -Wall -Wextra -Werror -s -Ofast -mwindows
CXXLIBS= -lsfml-network -lsfml-system
CXXLIBPATHS=

SRC = $(wildcard *.cpp)
SRH = $(wildcard *.hpp)
OBJ = $(patsubst %.cpp, make/%.o, $(SRC))

FINAL= server

$(FINAL): $(OBJ) 
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) $(CXXLIBPATHS) $(OBJ) $(CXXLIBS) -o $(FINAL)

make/%.o: %.cpp $(SRC) Makefile $(SRH)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@


clean: 
	@del .\make\*.o
