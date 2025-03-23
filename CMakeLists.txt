cmake_minimum_required(VERSION 3.5) # CMake install : https://cmake.org/download/
project(VideoPlayer LANGUAGES CXX)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "/usr/local/Qt/Qt6.5.3/6.5.3/gcc_64") # Qt Kit Dir
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(FFmpeg /usr/local/ffmpeg)

find_package(Qt6 COMPONENTS Widgets REQUIRED) # Qt COMPONENTS
find_package(SDL2 REQUIRED)

set(SOURCE 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/VideoPlayer.cpp
    ${CMAKE_SOURCE_DIR}/src/About.cpp
    ${CMAKE_SOURCE_DIR}/src/VideoPlayThread.cpp

)

set(INCLUDE
    ${CMAKE_SOURCE_DIR}/include/VideoPlayer.h
    ${CMAKE_SOURCE_DIR}/include/About.h
    ${CMAKE_SOURCE_DIR}/include/VideoPlayThread.h
)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_SOURCE_DIR}/ui)
set(UI_FILE
    ${CMAKE_SOURCE_DIR}/ui/VideoPlayer.ui
    ${CMAKE_SOURCE_DIR}/ui/About.ui
)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(${PROJECT_NAME} 
    WIN32 # If you need a terminal for debug, please comment this statement 
    ${SOURCE}
    ${INCLUDE}
    ${UI_FILE}
) 

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 添加库文件路径
link_directories(${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets

    # FFmpeg相关动态链接库
    avformat avdevice avcodec swresample swscale avutil

    # SDL2
    SDL2::SDL2
    SDL2::SDL2main
)