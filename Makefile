CXX= ccache g++
CC= gcc
CXXFLAGS= -std=c++20 -Wall -Wextra -Werror -s -O3 -DSFML_STATIC -Wpedantic -D_IRRIMGUI_STATIC_LIB_ -D_IRR_STATIC_LIB_ 
CFLAGS= -std=c99 -Wall -Wextra -Werror -s -Ofast -mwindows
CXXLIBS= -lirrlicht -lIrrIMGUI -limgui -lIrrAssimp -lassimp -lz -lsfcollider -limgui-SFML -limgui -lsfml-graphics-s -lsfml-window-s -ljpeg -lfreetype -lgdi32 -lsfml-audio-s -lopenal32 -lFLAC -lvorbisenc -lvorbisfile -lvorbis -logg -lsfml-network-s -lws2_32 -lsfml-system-s -lopengl32 -lwinmm 
CXXLIBPATHS= -L"C:\Users\famma\Documents\My_Programs\irrlicht\lib\Win32-gcc\static" -L"C:\Users\famma\Documents\My_Programs\SFML\SFML-2.6.x\extlibs\libs-mingw\x64" -L"C:\Users\famma\Documents\My_Programs\SFML\SFML-2.6.x\staticbuild\lib" -L"C:\Users\famma\Documents\My_Programs\Dear_ImGUI\lib" -L"C:\Users\famma\Documents\PATH" -L"C:\Users\famma\Documents\My_Programs\Dear_ImGUI\IrrIMGUI-0.3.1\build" -L"C:\Users\famma\Documents\My_Programs\assimp-master\build\lib" -L"C:\Users\famma\Documents\My_Programs\IrrAssimp-master\lib"

SRC = $(wildcard *.cpp)
SRH = $(wildcard *.hpp)
OBJ = $(patsubst %.cpp, make/%.o, $(SRC))
sSRC = $(wildcard server/*.cpp)

FINAL= car.exe

$(FINAL): $(OBJ) $(SRH) Makefile $(sSRC)
	@wsl ./make.sh
	@cd server && make -f Makefilewin
	@cd ..
	@windres icon.rc -o make/icon.o
	@echo "Linking $@"
	@$(CXX) $(CXXFLAGS) $(CXXLIBPATHS) $(OBJ) $(CXXLIBS) -o $(FINAL)

make/%.o: %.cpp $(SRC) Makefile $(SRH) $(sSRC)
	@echo "Compiling $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

./server/server:
	

clean: 
	@del .\make\*.o
