cmake_minimum_required(VERSION 3.0.2)
 
PROJECT(SPHFluidSim)

set(SPHFluidSim_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/CMake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${SPHFluidSim_CMAKE_DIR})

# Glob all the files together for use
file(GLOB MAIN_FILES *.h *.cpp)
file(GLOB SHADER_FILES *.glsl)

# Find the packages
add_subdirectory(GLEngine)

add_executable(SPHFluidSim WIN32 ${MAIN_FILES} ${SHADER_FILES})
target_include_directories(SPHFluidSim PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(SPHFluidSim PUBLIC GLEngine)

if(WIN32) # Check if we are on Windows
  if(MSVC) # Check if we are using the Visual Studio compiler
    # Turn off debug console for release config
    set_target_properties(SPHFluidSim PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
	set_target_properties(SPHFluidSim PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
	# Enable parallel build for msvc
	add_definitions(/MP)
  endif()
endif()

source_group("Shaders" FILES ${SHADER_FILES})

# Copy the compute shader
# Add a build command to copy the shaders to the build directory post build
add_custom_command(
	TARGET SPHFluidSim POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${SHADER_FILES}
			${CMAKE_BINARY_DIR}/Shaders)
add_custom_command(
	TARGET SPHFluidSim POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${SHADER_FILES}
			${CMAKE_BINARY_DIR}/Release/Shaders)
add_custom_command(
	TARGET SPHFluidSim POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
			${SHADER_FILES}
			${CMAKE_BINARY_DIR}/Debug/Shaders)