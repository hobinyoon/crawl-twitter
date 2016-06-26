CMAKE_MINIMUM_REQUIRED (VERSION 2.8 FATAL_ERROR)

SET (CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

# C++11
if (UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

FIND_PACKAGE( Boost
	COMPONENTS date_time filesystem program_options system timer REQUIRED )

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE ( tt
	conf.cpp
	main.cpp
	op.cpp
	util.cpp )
TARGET_LINK_LIBRARIES ( tt ${Boost_LIBRARIES} mysqlcppconn )
