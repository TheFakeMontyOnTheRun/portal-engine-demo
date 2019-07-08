cmake_minimum_required(VERSION 3.12)
project(portals)

set(CMAKE_CXX_STANDARD 14)

include_directories(game/include)
include_directories(game/OPL2LPT)


find_package(SDL REQUIRED)

add_executable(portals
        game/include/3D.h
        game/include/Engine.h
        game/include/FixP.h
        game/include/Graphics.h
        game/include/LoadBitmap.h
        game/include/Renderer.h
        game/include/SpyTravel.h
        game/include/Vec2i.h
        game/include/VisibilityStrategy.h
        game/src/3D.cpp
        game/src/BASIC.cpp
        game/src/CarMenu.cpp
        game/src/Crawler.cpp
        game/src/FixP.cpp
        game/src/Graphics.cpp
        game/src/LoadBitmap.cpp
        game/src/Main.cpp
        game/src/MainMenu.cpp
        game/src/RendererSDL.cpp
        game/src/SpyTravel.cpp
        game/src/Story.cpp
        game/src/Vec2i.cpp
        game/src/VisibilityStrategy.cpp)


find_package(OpenGL REQUIRED)

include (FindPkgConfig)

target_link_libraries(portals ${OPENGL_LIBRARIES} ${SDL_LIBRARY} )