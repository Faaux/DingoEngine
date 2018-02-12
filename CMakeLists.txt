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

endif()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Third Party Builds
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
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
		set(FREETYPE_LIBS ${CMAKE_SOURCE_DIR}/ThirdParty/freetype-2.9/freetype.lib)

	## SDL	
		SET (SDL_LIB_PATH_DEBUG ${CMAKE_SOURCE_DIR}/ThirdParty/SDL/lib/Debug)
		SET (SDL_LIB_PATH_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/SDL/lib/Release)


		set(SDL_LIBS
						debug ${SDL_LIB_PATH_DEBUG}/SDL2maind.lib
						debug ${SDL_LIB_PATH_DEBUG}/SDL2d.lib
						optimized ${SDL_LIB_PATH_RELEASE}/SDL2main.lib
						optimized ${SDL_LIB_PATH_RELEASE}/SDL2.lib
						)
		
	## PhsyX 3.4
		SET (PHYSX_LIB_PATH_DEBUG ${CMAKE_SOURCE_DIR}/ThirdParty/physx-3.4/lib/Debug)
		SET (PHYSX_LIB_AFFIX_DEBUG DEBUG_x64.lib)

		SET (PHYSX_LIB_PATH_RELEASE ${CMAKE_SOURCE_DIR}/ThirdParty/physx-3.4/lib/Release)
		SET (PHYSX_LIB_AFFIX_RELEASE CHECKED_x64.lib)

		set(PHSYX_LIBS 		
							optimized ${PHYSX_LIB_PATH_RELEASE}/PhysX3CharacterKinematic${PHYSX_LIB_AFFIX_RELEASE}
							optimized ${PHYSX_LIB_PATH_RELEASE}/PhysX3${PHYSX_LIB_AFFIX_RELEASE}
							optimized ${PHYSX_LIB_PATH_RELEASE}/PhysX3Common${PHYSX_LIB_AFFIX_RELEASE}
							optimized ${PHYSX_LIB_PATH_RELEASE}/PhysX3Cooking${PHYSX_LIB_AFFIX_RELEASE}
							optimized ${PHYSX_LIB_PATH_RELEASE}/PxFoundation${PHYSX_LIB_AFFIX_RELEASE}
							optimized ${PHYSX_LIB_PATH_RELEASE}/PxPvdSDK${PHYSX_LIB_AFFIX_RELEASE}	
							optimized ${PHYSX_LIB_PATH_RELEASE}/PhysX3ExtensionsCHECKED.lib

							debug ${PHYSX_LIB_PATH_DEBUG}/PhysX3CharacterKinematic${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PhysX3${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PhysX3Common${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PhysX3Cooking${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PxFoundation${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PxPvdSDK${PHYSX_LIB_AFFIX_DEBUG}
							debug ${PHYSX_LIB_PATH_DEBUG}/PhysX3ExtensionsDEBUG.lib
							)
		

		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	ThirdParty/SDL/include
									ThirdParty/glm
									ThirdParty/glad/include
									ThirdParty/imgui
									ThirdParty/imguiGizmo
									ThirdParty/tinyGltf
									ThirdParty/freetype-2.9
									ThirdParty/physx-3.4/Include
									ThirdParty/json
									)
		
		
		set(THIRD_PARTY_LIBS 		${SDL_LIBS} ${FREETYPE_LIBS} ${PHSYX_LIBS})
		
		set(THIRD_PARTY_SRC_FILES   ${GLAD_SOURCE_FILES}
									${IMGUI_SOURCE_FILES}
									${IMGUI_GIZMO_SOURCE_FILES})

#Includes
include_directories(src
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


# Copy PhysX Dlls
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/ThirdParty/physx-3.4/bin/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Copy Freetype Dll
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD        
    COMMAND ${CMAKE_COMMAND} -E copy_if_different			
        "${PROJECT_SOURCE_DIR}/ThirdParty/freetype-2.9/freetype.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)

# Copy SDL Dll
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/ThirdParty/SDL/bin/$<CONFIGURATION>"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)