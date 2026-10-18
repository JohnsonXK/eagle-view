cmake_minimum_required(VERSION 3.18)
project(EagleView CXX)
set(CMAKE_CXX_STANDARD 17)

add_library(EagleView SHARED src/EagleViewMod.cpp)
if(ANDROID)
  target_link_libraries(EagleView log)
endif()
