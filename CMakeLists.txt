cmake_minimum_required(VERSION 3.13)
project(Clonk3)

set(CONTENTDIR Content)

add_executable(clonk)

# Add SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(clonk ${SDL2_LIBRARIES})

# Make the code actually compile
target_compile_options(clonk PRIVATE -fpermissive)

add_subdirectory(Port)
add_subdirectory(Source)

target_include_directories(clonk
PRIVATE
	Port
)

# Copy content
add_custom_command(
		TARGET clonk POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${CONTENTDIR} $<TARGET_FILE_DIR:clonk>
)