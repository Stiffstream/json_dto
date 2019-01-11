add_library(json_dto_catch_main STATIC catch_main.cpp)
add_library(json-dto::catch_main ALIAS json_dto_catch_main)

TARGET_INCLUDE_DIRECTORIES(json_dto_catch_main PRIVATE ${CMAKE_SOURCE_DIR})
