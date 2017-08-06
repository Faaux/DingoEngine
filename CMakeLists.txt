# Force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

# Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.6)

# Project Name
project(DingoEngine)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set(CMAKE_C_FLAGS "/Zi /MP")
	set(CMAKE_C_FLAGS_DEBUG "-DDEBUG=1 /MTd /Od")
	set(CMAKE_C_FLAGS_RELEASE "/MT")
	set(CMAKE_C_FLAGS_RELWITHDEBINFO "/MT")
	set(CMAKE_CXX_FLAGS "/EHsc /Zi /MP")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG=1 /MTd /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "/MT")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	
	IF (DEFINED _DEV_USER)
		ADD_DEFINITIONS(-D_DEV_USER=${_DEV_USER})
	ENDIF()

	
endif()

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


# Third Party Builds
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
	## SDL	
		set(DIRECTX OFF CACHE BOOL "" FORCE)
		set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
		set(DISKAUDIO OFF CACHE BOOL "" FORCE)
		set(DUMMYAUDIO OFF CACHE BOOL "" FORCE)
		set(SDL_RENDER OFF CACHE BOOL "" FORCE)
		set(SDL_POWER ON CACHE BOOL "" FORCE)
		set(SDL_THREADS ON CACHE BOOL "" FORCE)
		set(SDL_FILE ON CACHE BOOL "" FORCE)
		set(SDL_SHARED OFF CACHE BOOL "" FORCE)
		set(SDL_STATIC ON CACHE BOOL "" FORCE)
		set(LIBC ON CACHE BOOL "" FORCE)
		add_subdirectory(ThirdParty/SDL-mirror)
		set_target_properties (SDL2-static PROPERTIES
			FOLDER ThirdParty/SDL
		)
		set_target_properties (SDL2main PROPERTIES
			FOLDER ThirdParty/SDL
		)
		set_target_properties (uninstall PROPERTIES
			FOLDER ThirdParty/SDL
		)

		set(SDL_LIBS
			SDL2-static
			SDL2main
		)
	
	## GLM
		# Nothing needed just added the includes to THIRD_PARTY_INCLUDES
		
	## GLEW
		set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
		add_subdirectory(ThirdParty/glew/build/cmake)
		set_target_properties (glew PROPERTIES
			FOLDER ThirdParty/glew
		)
		set_target_properties (glew_s PROPERTIES
			FOLDER ThirdParty/glew
		)
		
		set(GLEW_LIBS
			glew
		)
		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	${SDL2_SOURCE_DIR}/include
									ThirdParty/glm
									ThirdParty/glew/include)
		
		
		set(THIRD_PARTY_LIBS 		${SDL_LIBS}
									${GLEW_LIBS})
	

# Includes
include_directories(include ${THIRD_PARTY_INCLUDES})

#Source Files
set (SOURCE_FILES 	src/main.cpp
					src/DG_Job.cpp
					src/DG_SDLHelper.cpp
					src/DG_Camera.cpp)

#Header Files
set (HEADER_FILES	include/DG_Job.h
					include/DG_Include.h
					include/DG_SDLHelper.h
					include/DG_Windows.h
					include/DG_Camera.h
					include/DG_Mesh.h
					include/DG_Primitives.h
					include/DG_Shader.h
					include/DG_Transform.h
					)

#Shader Files
set(SHADERS shaders/fragment_shader.fs
			shaders/vertex_shader.vs)	

#Executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES} ${SHADERS})
source_group("shaders" FILES ${SHADERS})
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_LIBS})
target_compile_definitions(${PROJECT_NAME}	PRIVATE	SOURCEPATH=${CMAKE_CURRENT_LIST_DIR})