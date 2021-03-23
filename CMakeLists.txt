cmake_minimum_required(VERSION 3.13)
project(Clonk3)

option(GERMAN_CONTENT "Use German instead of English content files" OFF)
option(EASY_MONSTER_BBOX "Make monsters easier to hit by increasing their bounding box" ON)

# Enable address sanitizer
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# Path to content files
if (GERMAN_CONTENT)
	set(CONTENTDIR "content-de")
else()
	set(CONTENTDIR "content")
endif()

if (EASY_MONSTER_BBOX)
	add_compile_definitions(LARGER_MONSTER_BBOX)
endif()

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