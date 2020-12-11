cmake_minimum_required(VERSION 3.10)

project(chaq-tiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MINGW)
	add_compile_options(-flto)
endif()

add_subdirectory(./chaq-tiler)
add_subdirectory(./chaq-win-traits EXCLUDE_FROM_ALL)
add_subdirectory(./test-bench EXCLUDE_FROM_ALL)
