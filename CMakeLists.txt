cmake_minimum_required(VERSION 3.20)

project(snake_online VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED 11)

# Prefix for option names
set(PREFIX SNAKE)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(${PREFIX}_ENABLE_WARNING_CFLAGS "Enable warning flags" ON)
option(${PREFIX}_TURN_WARNINGS_INTO_ERRORS "Turn all warnings into errors" ON)
option(${PREFIX}_ENABLE_ADDRESS_SANITIZER "Enable address sanitizer" ON)
option(${PREFIX}_ENABLE_MEMORY_LEEKS_CHECK "Enable memory leaks check" ON)
option(ENABLE_SDL_VENDORED "Use vendored sdl (vendored/sdl)" OFF)
option(${PREFIX}_BUILD_TESTS "Build tests" ON)

# Suppress msvc crt secure warnings
if (MSVC)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

# Set warning flags
if (${PREFIX}_ENABLE_WARNING_CFLAGS)
    set(${PREFIX}_WARNING_CFLAGS "")

    if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
        set(${PREFIX}_WARNING_CFLAGS -Wall -Wextra)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(${PREFIX}_WARNING_CFLAGS /W4)
    endif()

    if (${PREFIX}_WARNING_CFLAGS)
        add_compile_options(${${PREFIX}_WARNING_CFLAGS})
    endif()
endif()

if (${PREFIX}_TURN_WARNINGS_INTO_ERRORS)
    set(${PREFIX}_WARNING_CFLAGS "")

    if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
        set(${PREFIX}_WARNING_CFLAGS -Werror)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(${PREFIX}_WARNING_CFLAGS /WX)
    endif()

    if (${PREFIX}_WARNING_CFLAGS)
        add_compile_options(${${PREFIX}_WARNING_CFLAGS})
    endif()
endif()

# Debugging and diagnostics
if (${PREFIX}_ENABLE_ADDRESS_SANITIZER)
    set(${PREFIX}_COMPILE_SAN_FLAGS "")
    set(${PREFIX}_LINKER_SAN_FLAGS "")
    if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
        set(${PREFIX}_COMPILE_SAN_FLAGS -fsanitize=address)
        set(${PREFIX}_LINKER_SAN_FLAGS -fsanitize=address)
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        set(${PREFIX}_COMPILE_SAN_FLAGS /fsanitize=address)
    endif()

    if (${PREFIX}_COMPILE_SAN_FLAGS)
        add_compile_options(${${PREFIX}_COMPILE_SAN_FLAGS})
    endif()

    if (${PREFIX}_LINKER_SAN_FLAGS)
        add_link_options(${${PREFIX}_LINKER_SAN_FLAGS})
    endif()
endif()

if (${PREFIX}_ENABLE_MEMORY_LEEKS_CHECK)
    if ((CMAKE_C_COMPILER_ID STREQUAL "Clang") OR (CMAKE_C_COMPILER_ID STREQUAL "GNU"))
        if (NOT ${PREFIX}_ENABLE_ADDRESS_SANITIZER)
            add_compile_options(-fsanitize=address)
            add_link_options(-fsanitize=address)
        endif()
    elseif (CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_compile_definitions(_CRTDBG_MAP_ALLOC)
    endif()
endif()

# Add targets
# SDL2
if(ENABLE_SDL_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    find_package(SDL2 CONFIG REQUIRED)
endif()

# SDL2_image
find_package(SDL2_image CONFIG REQUIRED)

# SDL2_ttf
find_package(SDL2_ttf CONFIG REQUIRED)


# snake_common
add_library(snake_common STATIC
                            src/common/protocol.c
                            src/common/game.c
                            src/common/snake.c
                            src/common/color.c)
target_include_directories(snake_common
                                PUBLIC include)
target_compile_definitions(snake_common PRIVATE
                           $<$<CONFIG:Debug>:DEBUG_NETWORK>)

# snake_client library
add_library(snake_client STATIC 
                            src/client/render.c
                            src/client/network.c
                            src/client/input.c
                            src/client/client.c
                            src/client/menu.c)
target_include_directories(snake_client 
                                PUBLIC include
                                PRIVATE src/client)
target_link_libraries(snake_client PUBLIC 
                                        SDL2::SDL2 
                                        SDL2_image::SDL2_image
                                        SDL2_ttf::SDL2_ttf
                                        snake_common)

# client executable
add_executable(client src/client/main.c)
target_link_libraries(client PRIVATE snake_client)

# snake_server library
add_library(snake_server STATIC
                            src/server/server.c
                            src/server/input_queue.c)
target_include_directories(snake_server 
                                PUBLIC include
                                PRIVATE src/server)
target_link_libraries(snake_server PUBLIC
                                        snake_common)

# server executable
add_executable(server src/server/main.c)
target_link_libraries(server PRIVATE snake_server)

if ($ENV{CMAKE_EXPORT_COMPILE_COMMANDS})
    execute_process(
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
    )
endif()

# Tests
if (${PREFIX}_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

