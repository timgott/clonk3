cmake_minimum_required(VERSION 3.13)
project(Clonk3)

# Path to content files
set(CONTENTDIR content)

add_executable(clonk)

# Add SDL2
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(clonk PRIVATE SDL2::SDL2 SDL2::SDL2main)

# Required options to make the code actually compile
if(CMAKE_COMPILER_IS_GNUCC)
	target_compile_options(clonk PRIVATE -fpermissive)
endif()

# Source directories
add_subdirectory(src)

# Copy content
add_custom_command(
		TARGET clonk POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/${CONTENTDIR} $<TARGET_FILE_DIR:clonk>
)