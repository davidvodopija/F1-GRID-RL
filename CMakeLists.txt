cmake_minimum_required(VERSION 3.20)

project(F1GridWorld VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Nice! Using Clang.")
    add_compile_options(
        -Wall 
        -Wextra 
        -Wpedantic 
        -Wno-unused-parameter
    )
elseif(MSVC)
    add_compile_options(/W4)
endif()

include(FetchContent)

message(STATUS "Fetching nlohmann/json...")
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

message(STATUS "Fetching SFML...")
FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.2
)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(sfml)


file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.hpp")

add_executable(F1Sim ${SOURCES})

target_link_libraries(F1Sim PRIVATE nlohmann_json::nlohmann_json sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET F1Sim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:F1Sim>/assets
)