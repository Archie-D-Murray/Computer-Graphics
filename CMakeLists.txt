# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Computer_Graphics_Coursework)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory!" )
endif()

# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

include_directories(
	external/glfw-3.1.2/include/
	external/glew-1.13.0/include/
	external/glm-0.9.7.1/
	external/freetype-2.13.3/include/
	.
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLEW_1130
	freetype
)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

# ==============================================================================
add_executable(Computer_Graphics_Coursework
	source/coursework.cpp
	source/vertexShader.glsl
	source/fragmentShader.glsl

	common/shader.hpp
	common/box_collider2d.cpp
	common/object.cpp
	common/shader.cpp
	common/texture.hpp
	common/stb_image.hpp
	common/maths.hpp
	common/maths.cpp
	common/camera.hpp
	common/camera.cpp
	common/model.hpp
	common/model.cpp
	common/light.hpp
	common/light.cpp

)
target_link_libraries(Computer_Graphics_Coursework
	${ALL_LIBS}
)

# Xcode and Visual working directories
set_target_properties(Computer_Graphics_Coursework PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/")
create_target_launcher(Computer_Graphics_Coursework WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/source/")
create_default_target_launcher(Computer_Graphics_Coursework WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/source/") 

# ==============================================================================
if (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

add_custom_command(
   TARGET Computer_Graphics_Coursework POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Computer_Graphics_Coursework${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/source/"
)

elseif (${CMAKE_GENERATOR} MATCHES "Xcode" )

endif (NOT ${CMAKE_GENERATOR} MATCHES "Xcode" )

