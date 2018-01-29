# Force out-of-source build
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

# Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.9)

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
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)	
	add_definitions(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
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
		set(VIDEO_OPENGLES OFF CACHE BOOL "" FORCE)
		set(VIDEO_DUMMY OFF CACHE BOOL "" FORCE)
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
									ThirdParty/imgui/imgui_draw.cpp
									)
	## IMGUI_GIZMO
		set (IMGUI_GIZMO_SOURCE_FILES 	ThirdParty/imguiGizmo/ImGuizmo.cpp
										ThirdParty/imguiGizmo/ImSequencer.cpp
										)
									
	## GLAD
		set (GLAD_SOURCE_FILES 		ThirdParty/glad/src/glad.c)
		
	## FreeType
		
		
		set(FREETYPE_LIBS bin/freetype)
		configure_file(ThirdParty/freetype-2.9/freetype.lib bin/freetype.lib COPYONLY)
		configure_file(ThirdParty/freetype-2.9/freetype.dll bin/freetype.dll COPYONLY)
		
		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	${SDL2_SOURCE_DIR}/include
									ThirdParty/glm
									ThirdParty/glad/include
									ThirdParty/imgui
									ThirdParty/imguiGizmo
									ThirdParty/tinyGltf
									ThirdParty/freetype-2.9
									ThirdParty/stb
									)
		
		
		set(THIRD_PARTY_LIBS 		${SDL_LIBS} ${FREETYPE_LIBS})
		
		set(THIRD_PARTY_SRC_FILES   ${GLAD_SOURCE_FILES}
									${IMGUI_SOURCE_FILES}
									${IMGUI_GIZMO_SOURCE_FILES})
	

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
					src/DG_Messaging.cpp
					src/DG_ResourceManager.cpp
					src/DG_StringId.cpp
					src/DG_memory.cpp
					
					src/graphics/DG_Font.cpp
					src/graphics/DG_Texture.cpp
					src/graphics/DG_GraphicsSystem.cpp
					src/graphics/DG_Shader.cpp
					src/graphics/DG_Mesh.cpp
					
					src/misc/DG_Clock.cpp
					src/misc/DG_SDLHelper.cpp
					src/misc/imgui_impl_sdl_gl3.cpp
					)

#Header Files
set (HEADER_FILES	src/DG_Job.h
					src/DG_Camera.h
					src/DG_Transform.h
					src/DG_InputSystem.h
					src/DG_ResourceHelper.h
					src/DG_Messaging.h
					src/DG_ResourceManager.h
					src/DG_StringId.h
					src/DG_memory.h

					src/graphics/DG_Font.h					
					src/graphics/DG_Texture.h
					src/graphics/DG_GraphicsSystem.h
					src/graphics/DG_Shader.h
					src/graphics/DG_Mesh.h
					
					src/misc/DG_Include.h
					src/misc/DG_Clock.h
					src/misc/DG_SDLHelper.h
					src/misc/DG_Windows.h
					src/misc/imgui_impl_sdl_gl3.h
					)

#Shader Files
set(SHADERS shaders/fragment_shader.fs
			shaders/vertex_shader.vs
			
			shaders/billboard_font.fs
			shaders/billboard_font.vs
			shaders/screen_font.fs
			shaders/screen_font.vs
			
			shaders/vertex_shader_debug_lines.vs
			shaders/fragment_shader_debug_lines.fs
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
