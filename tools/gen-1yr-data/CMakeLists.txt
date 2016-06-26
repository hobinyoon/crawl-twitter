CMAKE_MINIMUM_REQUIRED (VERSION 2.8 FATAL_ERROR)

SET (CMAKE_BUILD_TYPE Release)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pthread")

# C++11
if (UNIX)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
endif()

FIND_PACKAGE( Boost
	COMPONENTS date_time filesystem program_options system timer REQUIRED )

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
ADD_EXECUTABLE ( gen-1yr-data
	conf.cpp
	main.cpp
	tweet.cpp
	util.cpp
	youtube-dataset.cpp )
TARGET_LINK_LIBRARIES ( gen-1yr-data ${Boost_LIBRARIES} )
