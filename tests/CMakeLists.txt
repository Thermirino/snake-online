add_library(unity STATIC unity/src/unity.c unity/src/unity.h unity/src/unity_internals.h)
target_include_directories(unity PUBLIC unity/src)
target_compile_definitions(unity PUBLIC -DUNITY_INCLUDE_DOUBLE)

add_executable(test_render test_render.c)
target_link_libraries(test_render PRIVATE unity snake_client)
target_include_directories(test_render PRIVATE ${PROJECT_SOURCE_DIR}/src/client) 

# Copy assets
add_custom_command(TARGET test_render POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/tests/assets)

add_test(NAME test_render COMMAND test_render)
