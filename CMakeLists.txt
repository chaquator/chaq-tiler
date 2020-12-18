cmake_minimum_required(VERSION 3.10)

project(chaq-tiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Release)

if(MINGW)
	add_compile_options(-flto)
	add_compile_options(-municode)
endif()

add_subdirectory(./chaq-tiler)
add_subdirectory(./chaq-win-traits)
add_subdirectory(./test-bench)
