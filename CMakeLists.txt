cmake_minimum_required(VERSION 3.16)
project(ElkBledomControl VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(simpleble REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
set(SOURCES 
    main.cpp 
    commands.cpp
)
pkg_check_modules(FFTW3 REQUIRED fftw3f)
pkg_check_modules(RTAUDIO REQUIRED rtaudio)
add_executable(elk_led ${SOURCES})
target_link_libraries(elk_led PRIVATE ${FFTW3_LIBRARIES} ${RTAUDIO_LIBRARIES} simpleble::simpleble)
find_package(Threads REQUIRED)
target_link_libraries(elk_led PRIVATE ${FFTW3_LIBRARIES} ${RTAUDIO_LIBRARIES} Threads::Threads)