add_executable(suite_1_app
    suite_1.c
)

target_link_libraries(suite_1_app
    gpib_parse
    Unity
)

add_test(NAME suite_1_test COMMAND suite_1_app)
