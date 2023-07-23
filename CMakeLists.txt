cmake_minimum_required(VERSION 3.10)
project(VulkanTutorial)

# Threads
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Graphics libraries
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	add_definitions(-DDEBUG="1")
endif()

set(SOURCE_FILES main.cpp)
set(EXECUTABLE vulkan-tutorial)
add_executable(${EXECUTABLE} ${SOURCE_FILES})

set_target_properties(${EXECUTABLE} PROPERTIES
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
)

target_link_libraries(${EXECUTABLE} PUBLIC
	Threads::Threads

	Vulkan::Vulkan
	glfw
)

if(MSVC)
  target_compile_options(${EXECUTABLE} PRIVATE /W4 /WX)
else()
  target_compile_options(${EXECUTABLE} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()
