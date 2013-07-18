cmake_minimum_required(VERSION 2.8)
include(CheckIncludeFile)

project(lshidraw)


check_include_file("linux/hidraw.h" HAVE_HIDRAW)
if (NOT HAVE_HIDRAW)
	message(FATAL_ERROR "Could not locate <linux/hidraw.h>")
endif()

add_executable(lshidraw lshidraw.c hexdump.c)

add_executable(dumphidraw dumphidraw.c hexdump.c)
if (NOT ANDROID)
	target_link_libraries(dumphidraw rt)
endif()

