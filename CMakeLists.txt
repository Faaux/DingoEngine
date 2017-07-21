# Force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

# Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.6)

# Project Name
project(DingoEngine)

# C++ Version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std:c++11")

# Includes
include_directories(include
					ThirdParty/include)
					
# Link Libs
link_directories(ThirdParty/lib)

# Runtime folders for VS

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)


#Source Files
set (SOURCE_FILES 	src/main.cpp)

#Header Files
set (HEADER_FILES	)

#Executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${PROJECT_NAME} SDL/SDL2)
target_link_libraries(${PROJECT_NAME} SDL/SDL2main)

#Copy DLLs to working dir
IF (MSVC)
	file(COPY ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/binaries/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/.)
ELSE()

ENDIF()