cmake_minimum_required(VERSION 3.15)

find_package(Boost COMPONENTS system thread REQUIRED)

set(CURSES_NEED_WIDE TRUE)
find_package(Curses REQUIRED)
message(STATUS "CURSES_LIBRARIES: ${CURSES_LIBRARIES}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/)
set(TARGET_SOURCES
  wumpus.cpp
  Game.cpp
  World.cpp
)

add_compile_definitions(_LINUX)
include_directories(
  ${PROJECT_SOURCE_DIR}/include/
  ${PROJECT_SOURCE_DIR}/../simple-game-lib/os-terminal/include/
  ${CURSES_INCLUDE_DIR}
)
link_directories(${PROJECT_SOURCE_DIR}/../simple-game-lib/os-terminal/lib)
add_executable(${PROJECT_NAME} ${TARGET_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
  Boost::boost
  Boost::system
  Boost::thread
  sgl-os-terminal
  ${CURSES_LIBRARIES}
)
