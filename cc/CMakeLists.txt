cmake_minimum_required(VERSION 2.8)
project(c0)

enable_language(C)

if (MSVC)
	add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif()

include_directories("${PROJECT_SOURCE_DIR}")
set(SRCS c00.c c01.c c02.c c03.c c04.c c05.c)
set(HEADERS c0.h)

add_executable(c0 ${SRCS} ${HEADERS})
