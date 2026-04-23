cmake_minimum_required(VERSION 3.10)

project("ArmorHUD")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Ofast")

add_library(ArmorHUD SHARED
    main.cpp
)

target_link_libraries(ArmorHUD
    android
    log
    dl
)
