cmake_minimum_required(VERSION 3.15)
project(LexicalAnalyzerGenerator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core lexer library (Step 0 & Step 1)
add_library(lexer_core
    src/RegexToken.cpp
    src/RuleParser.cpp
    src/NFA.cpp
    src/NFABuilder.cpp
)

# Test executable for Step 0
add_executable(test_step0 tests/test_step0.cpp)
target_link_libraries(test_step0 PRIVATE lexer_core)

# Test executable for Step 1
add_executable(test_step1 tests/test_step1.cpp)
target_link_libraries(test_step1 PRIVATE lexer_core)

# Copy rules file to build directory for out-of-source execution
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/lexical_rules.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/lexical_rules.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests)
