
add_library(gpib_parse STATIC gpib_parse.c)

target_include_directories(gpib_parse PUBLIC ${CMAKE_CURRENT_LIST_DIR})
