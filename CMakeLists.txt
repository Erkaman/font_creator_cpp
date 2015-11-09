######################################
#### PROJECT SETTINGS.
######################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(font_creator_cpp)

SET(CMAKE_BUILD_TYPE "Debug")

######################################
#### SET SOURCE FILES.
######################################

file(GLOB SRC src/*.cpp)


######################################
############ ADD  LIBRARIES
######################################


find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})

include_directories("src")

######################################
############ MAKE EXECUTABLE
######################################


add_executable (font_creator_cpp ${SRC})
target_link_libraries(font_creator_cpp ${FREETYPE_LIBRARIES})
