cmake_minimum_required(VERSION 2.8)
project(filter)

include_directories(src/cache)

file(GLOB_RECURSE sources src/*.cpp src/cache/*.cpp)

add_executable(filter ${sources})

target_compile_options(filter PUBLIC -Wall -Werror -std=gnu++11)

# http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake/20165220#20165220
set_property(TARGET filter PROPERTY CXX_STANDARD 11)

find_package(Boost COMPONENTS
	date_time
	filesystem
	program_options
	regex
	system
	timer
	REQUIRED)

target_link_libraries(filter PUBLIC
	${Boost_LIBRARIES}
	)
