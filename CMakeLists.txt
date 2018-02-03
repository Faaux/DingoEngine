#Force out - of - source build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

#Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.9)

#Project Name
project(DingoEngine)

#Helper Function
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

#Flags
set (CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)	
	add_definitions(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	# Replace /MD with /MT
	foreach(flag_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
		CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

      if(${flag_var} MATCHES "/MD")
        string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
      endif()

    endforeach()

endif()

#Runtime folders for VS
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 			${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE 	${CMAKE_BINARY_DIR}/bin)

#Third Party Builds
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
		set(SDL_SHARED ON CACHE BOOL "" FORCE)
		set(SDL_STATIC OFF CACHE BOOL "" FORCE)
		set(LIBC ON CACHE BOOL "" FORCE)
		set(FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)
		
		add_subdirectory(ThirdParty/SDL-mirror)
		
		set_target_properties (SDL2 PROPERTIES
			FOLDER ThirdParty/SDL
		)
		set_target_properties (SDL2main PROPERTIES
			FOLDER ThirdParty/SDL
		)
		set_target_properties (uninstall PROPERTIES
			FOLDER ThirdParty/SDL
		)

		set(SDL_LIBS
			SDL2
			SDL2main
		)
	
	## GLM
#Nothing needed just added the includes to THIRD_PARTY_INCLUDES
		
	## IMGUI
		set (IMGUI_SOURCE_FILES 	ThirdParty/imgui/imgui.cpp
									ThirdParty/imgui/imgui_draw.cpp
									ThirdParty/imgui/imgui_demo.cpp
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
		
	## PhsyX 3.4
		SET (PHYSX_LIB_PATH ${CMAKE_SOURCE_DIR}/ThirdParty/physx-3.4/libDebug)
		SET (PHYSX_LIB_AFFIX DEBUG_x64.lib)

		set(PHSYX_LIBS 		
							${PHYSX_LIB_PATH}/PhysX3CharacterKinematic${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PhysX3${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PhysX3Common${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PhysX3Cooking${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PxFoundation${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PxPvdSDK${PHYSX_LIB_AFFIX}
							${PHYSX_LIB_PATH}/PhysX3ExtensionsDEBUG.lib # this is annoying...
							)

		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	${SDL2_SOURCE_DIR}/include
									ThirdParty/glm
									ThirdParty/glad/include
									ThirdParty/imgui
									ThirdParty/imguiGizmo
									ThirdParty/tinyGltf
									ThirdParty/freetype-2.9
									ThirdParty/stb
									ThirdParty/physx-3.4/Include
									)
		
		
		set(THIRD_PARTY_LIBS 		${SDL_LIBS} ${FREETYPE_LIBS} ${PHSYX_LIBS})
		
		set(THIRD_PARTY_SRC_FILES   ${GLAD_SOURCE_FILES}
									${IMGUI_SOURCE_FILES}
									${IMGUI_GIZMO_SOURCE_FILES})

#Includes
include_directories(src 
					src/misc 
					src/graphics
					${THIRD_PARTY_INCLUDES})

#Source Files
file(GLOB_RECURSE SOURCES
    src/*.h
    src/*.cpp
)

file(GLOB_RECURSE SHADERS
    shaders/*.vs
    shaders/*.fs
    shaders/*.gs
)

set(ALL_SOURCE_FILES 	${SOURCES} 
						${SHADERS}
						${THIRD_PARTY_SRC_FILES})
			
#Executable
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

# Create Source Groups based on path
assign_source_group(${ALL_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_LIBS})
target_compile_definitions(${PROJECT_NAME}	PRIVATE	SOURCEPATH=${CMAKE_CURRENT_LIST_DIR})
