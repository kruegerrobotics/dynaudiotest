project (dynamicaudio)
cmake_minimum_required(VERSION 3.5)

#test with portaudio and libsndfile
add_executable(test1 src/test1.cpp)
target_link_libraries(test1 portaudio sndfile)

#test with open AL soft mostly from internet
add_executable(test2 src/test2.cpp src/common/alhelpers.c)
include_directories(src/common)
target_link_libraries(test2 openal SDL_sound)

#example rebuild for application and reuse
add_executable(practical src/practical1.cpp )
target_link_libraries(practical openal SDL_sound ncurses pthread)