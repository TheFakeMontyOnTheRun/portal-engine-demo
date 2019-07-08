cmake_minimum_required(VERSION 3.5)
set(GAME_NAME portals)
project(${GAME_NAME})

set(CMAKE_CXX_STANDARD          14)
set(CMAKE_CXX_STANDARD_REQUIRED on)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_find")
find_package(SDL2 REQUIRED)

include_directories(
		${SDL2_INCLUDE_DIR}
)

include_directories(
		game/include
)
set(SOURCE_FILES
		game/src/3D.cpp
		game/src/Crawler.cpp
		game/src/FixP.cpp
		game/src/Graphics.cpp
		game/src/LoadBitmap.cpp
		game/src/Main.cpp
		game/src/MainMenu.cpp
		game/src/RendererSDL2.cpp
		game/src/Vec2i.cpp

		)

add_executable(${GAME_NAME} ${SOURCE_FILES})
target_include_directories(${GAME_NAME} PUBLIC ${SDL2_INCLUDE_DIR})
target_link_libraries(${GAME_NAME} LINK_PUBLIC ${SDL2_LIBRARY} m)
