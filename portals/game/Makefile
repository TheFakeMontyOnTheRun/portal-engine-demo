CC=clang
CXX=clang++
CFLAGS=-Iinclude -g -std=c11 `sdl2-config --cflags ` -fpermissive 
CXXFLAGS=-Iinclude -g -std=c++14 `sdl2-config --cflags ` -fpermissive
TARGET=game
LDFLAGS = `sdl2-config --libs` -lm

OBJ=	src/Main.o \
	src/Graphics.o \
	src/RendererSDL2.o  \
	src/FixP.o \
	src/MainMenu.o \
	src/3D.o \
	src/Vec2i.o \
	src/Crawler.o \
	src/LoadBitmap.o

$(TARGET): $(OBJ)
	$(CC) -o$(TARGET) $(OBJ) $(LDFLAGS)

all: $(TARGET)


clean:
	rm -f src/*.o
	rm -f $(TARGET)
