cmake_minimum_required(VERSION 3.7)

project(tomato-automata)

set(SRC ./src)
set(SRC_LIST
    ${SRC}/main.cpp
    ${SRC}/app.cpp ${SRC}/app.h
    ${SRC}/common.cpp ${SRC}/common.h
    ${SRC}/automata/automata.h ${SRC}/automata/life.cpp
    ${SRC}/automata/generations.cpp ${SRC}/automata/cyclic.cpp
    ${SRC}/automata/larger_than_life.cpp ${SRC}/automata/neumann_binary.cpp
    ${SRC}/automata/rules_table.cpp ${SRC}/automata/weighted_life.cpp
)
#aux_source_directory(./src SRC_LIST)

add_executable(${PROJECT_NAME} ${SRC_LIST})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)

set(GCC_COMPILER_FLAGS "-ggdb -Wall -Wextra")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COMPILER_FLAGS}")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(imgui
    # Main imgui files
    ./imgui/imgui.cpp ./imgui/imgui_draw.cpp ./imgui/imgui_widgets.cpp
    # imgui-SDL files
    ./imgui/imgui_sdl.h ./imgui/imgui_sdl.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})

# Make SDL2 available to Imgui
target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})

# imgui/examples contains the sdl implementation
target_include_directories(imgui PUBLIC ./imgui)

target_link_libraries(${PROJECT_NAME} imgui)
target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
