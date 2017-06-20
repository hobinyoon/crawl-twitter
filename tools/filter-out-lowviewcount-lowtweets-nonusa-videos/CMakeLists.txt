CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(filter)

FILE(GLOB_RECURSE sources src/*.cpp)

ADD_EXECUTABLE(filter ${sources})

TARGET_COMPILE_OPTIONS(filter PUBLIC -Wall -Werror -std=gnu++11 -pthread)

# http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake/20165220#20165220
SET_PROPERTY(TARGET filter PROPERTY CXX_STANDARD 11)

FIND_PACKAGE(Boost COMPONENTS
	date_time
	filesystem
	program_options
	regex
	system
	timer
	REQUIRED)

TARGET_LINK_LIBRARIES(filter PUBLIC
	${Boost_LIBRARIES}
	pthread
	)
