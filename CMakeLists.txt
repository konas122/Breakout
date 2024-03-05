cmake_minimum_required(VERSION 3.10)

project(main)

file(COPY resources/ DESTINATION ${CMAKE_BINARY_DIR}/Debug)
file(COPY source/shaders DESTINATION ${CMAKE_BINARY_DIR}/Debug)

set(SOURCE_DIR "source")
message("SOURCE_DIR: ${SOURCE_DIR}")
include_directories(${SOURCE_DIR})

file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp" "${SOURCE_DIR}/*.h")
# message("SOURCE_DIR: ${SOURCE_FILES}")
add_executable(main ${SOURCE_FILES})


find_package(glad CONFIG REQUIRED)
target_link_libraries(main PRIVATE glad::glad)
if(glad_FOUND)
    message("glad found")
endif()

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(main PRIVATE glfw)
if(glfw3_FOUND)
    message("glfw3 found")
endif()

find_package(glm CONFIG REQUIRED)
target_link_libraries(main PRIVATE glm::glm)
if(glm_FOUND)
    message("glm found")
endif()
