cmake_minimum_required(VERSION 3.15)
project(LexicalAnalyzerGenerator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# Core lexer library (Steps 0, 1, 2 & 3)
add_library(lexer_core
    src/RegexToken.cpp
    src/RuleParser.cpp
    src/NFA.cpp
    src/NFABuilder.cpp
    src/DFA.cpp
    src/DFABuilder.cpp
    src/DFAMinimizer.cpp
)

# Test executable for Step 0
add_executable(test_step0 tests/test_step0.cpp)
target_link_libraries(test_step0 PRIVATE lexer_core)

# Test executable for Step 1
add_executable(test_step1 tests/test_step1.cpp)
target_link_libraries(test_step1 PRIVATE lexer_core)

# Test executable for Step 2
add_executable(test_step2 tests/test_step2.cpp)
target_link_libraries(test_step2 PRIVATE lexer_core)

# Test executable for Step 3
add_executable(test_step3 tests/test_step3.cpp)
target_link_libraries(test_step3 PRIVATE lexer_core)

# Copy rules file to build directory for out-of-source execution
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/lexical_rules.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tests/lexical_rules.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests)