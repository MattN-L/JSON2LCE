cmake_minimum_required(VERSION 3.8)
project(JSON2LCE)

add_executable(JSON2LCE JSON2LCE.cpp)

target_include_directories(JSON2LCE PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/SDL/include
    ${CMAKE_CURRENT_LIST_DIR}/json/include
)

target_compile_definitions(JSON2LCE PRIVATE SDL_MAIN_USE_CALLBACKS=1)

set(SDL3_BUILD_SHARED ON CACHE BOOL "Build SDL3 shared library" FORCE)
set(SDL3_BUILD_STATIC OFF CACHE BOOL "Build SDL3 static library" FORCE)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/SDL)

target_link_libraries(JSON2LCE PRIVATE SDL3::SDL3-shared)

set_property(TARGET JSON2LCE PROPERTY CXX_STANDARD 23)

set(SDL3_DLL "${CMAKE_BINARY_DIR}/SDL/SDL3.dll")

add_custom_command(TARGET JSON2LCE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL3_DLL}"
    $<TARGET_FILE_DIR:JSON2LCE>
)