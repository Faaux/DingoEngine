# Force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

# Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.10)

# Project Name
project(DingoEngine)

# Helper Function
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# Flags
set (CMAKE_CXX_STANDARD 17)

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
		
	## IMGUI
		set (IMGUI_SOURCE_FILES 	ThirdParty/imgui/imgui.cpp
									ThirdParty/imgui/imgui_draw.cpp)
									
	## GLAD
		set (GLAD_SOURCE_FILES 		ThirdParty/glad/src/glad.c)
		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	${SDL2_SOURCE_DIR}/include
									ThirdParty/glm
									ThirdParty/glad/include
									ThirdParty/imgui
									ThirdParty/tinyGltf)
		
		
		set(THIRD_PARTY_LIBS 		${SDL_LIBS})
		
		set(THIRD_PARTY_SRC_FILES   ${GLAD_SOURCE_FILES}
									${IMGUI_SOURCE_FILES})
	

# Includes
include_directories(src 
					src/misc 
					src/graphics
					${THIRD_PARTY_INCLUDES})

#Source Files
set (SOURCE_FILES 	src/main.cpp
					src/DG_Job.cpp
					src/DG_Camera.cpp
					src/DG_InputSystem.cpp
					src/DG_ResourceHelper.cpp
					
					src/graphics/DG_GraphicsSystem.cpp
					src/graphics/DG_Shader.cpp
					src/graphics/DG_Mesh.cpp
					
					src/misc/DG_Profiler.cpp
					src/misc/DG_Clock.cpp
					src/misc/DG_SDLHelper.cpp
					src/misc/imgui_impl_sdl_gl3.cpp
					)

#Header Files
set (HEADER_FILES	src/DG_Job.h
					src/DG_Camera.h
					src/DG_Primitives.h
					src/DG_Transform.h
					src/DG_InputSystem.h
					src/DG_ResourceHelper.h
					
					src/graphics/DG_GraphicsSystem.h
					src/graphics/DG_Shader.h
					src/graphics/DG_Mesh.h
					
					src/misc/DG_Include.h
					src/misc/DG_Profiler.h					
					src/misc/DG_Clock.h
					src/misc/DG_SDLHelper.h
					src/misc/DG_Windows.h
					src/misc/imgui_impl_sdl_gl3.h
					)

#Shader Files
set(SHADERS shaders/fragment_shader.fs
			shaders/vertex_shader.vs
			)	

set(ALL_SOURCE_FILES 	${SOURCE_FILES}
						${HEADER_FILES} 
						${SHADERS}
						${THIRD_PARTY_SRC_FILES})
			
#Executable
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

# Create Source Groups based on path
assign_source_group(${ALL_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_LIBS})
target_compile_definitions(${PROJECT_NAME}	PRIVATE	SOURCEPATH=${CMAKE_CURRENT_LIST_DIR})