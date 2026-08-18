cmake_minimum_required(VERSION 3.20)
project(GoTo_SDK VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Warning flags
if(MSVC)
    add_compile_options(/W4 /utf-8 /EHsc)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Core GoTo Engine & SDK Library
set(GOTO_LIB_SOURCES
    src/value.cpp
    src/token.cpp
    src/lexer.cpp
    src/ast.cpp
    src/parser.cpp
    src/environment.cpp
    src/interpreter.cpp
    src/sdk.cpp
)

set(GOTO_LIB_HEADERS
    include/goto/common.hpp
    include/goto/value.hpp
    include/goto/token.hpp
    include/goto/lexer.hpp
    include/goto/ast.hpp
    include/goto/parser.hpp
    include/goto/environment.hpp
    include/goto/engine_events.hpp
    include/goto/interpreter.hpp
    include/goto/sdk.hpp
)

add_library(goto_core STATIC ${GOTO_LIB_SOURCES} ${GOTO_LIB_HEADERS})
target_include_directories(goto_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# CLI Executable
add_executable(goto_cli src/main.cpp)
target_link_libraries(goto_cli PRIVATE goto_core)
set_target_properties(goto_cli PROPERTIES OUTPUT_NAME "goto")

# Test Executable
set(GOTO_TEST_SOURCES
    tests/test_main.cpp
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_values.cpp
    tests/test_control_flow.cpp
    tests/test_narrative.cpp
)

add_executable(goto_tests ${GOTO_TEST_SOURCES})
target_link_libraries(goto_tests PRIVATE goto_core)

enable_testing()
add_test(NAME GoToTests COMMAND goto_tests)
