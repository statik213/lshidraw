cmake_minimum_required(VERSION 2.8)
include(CheckIncludeFile)

project(lshidraw)


check_include_file("linux/hidraw.h" HAVE_HIDRAW)
if (NOT HAVE_HIDRAW)
	message(FATAL_ERROR "Could not locate <linux/hidraw.h>")
endif()

add_executable(dumpdev dumpdev.c util.c)
add_executable(lshidraw lshidraw.c util.c)
add_executable(polldev polldev.c util.c)

if (NOT ANDROID)
	target_link_libraries(dumpdev rt)
	target_link_libraries(lshidraw rt)
	target_link_libraries(polldev rt)
endif()
