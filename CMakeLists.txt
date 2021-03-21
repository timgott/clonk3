cmake_minimum_required(VERSION 3.13)
project(Clonk3)

# Path to content files
set(CONTENTDIR content)

add_executable(clonk)

# Add SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(clonk ${SDL2_LIBRARIES})

# Source directories
add_subdirectory(src)

# Copy content
add_custom_command(
		TARGET clonk POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${CONTENTDIR} $<TARGET_FILE_DIR:clonk>
)