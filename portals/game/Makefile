CC=clang
CXX=clang++
CFLAGS=-Iinclude -g -std=c11 `sdl-config --cflags ` -fpermissive 
CXXFLAGS=-Iinclude -g -std=c++14 `sdl-config --cflags ` -fpermissive
TARGET=game
LDFLAGS = `sdl-config --libs` -lm

OBJ=	src/Main.o \
	src/Graphics.o \
	src/RendererSDL.o  \
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
