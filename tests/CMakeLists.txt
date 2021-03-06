project(ColorMyConsoleTests)

add_executable(colmc_test_colors)
set_property(TARGET colmc_test_colors PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_colors PRIVATE src/test_colors.cpp)
target_link_libraries(colmc_test_colors colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_colors PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_colors PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_raw_input)
set_property(TARGET colmc_test_raw_input PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_raw_input PRIVATE src/test_raw_input.cpp)
target_link_libraries(colmc_test_raw_input colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_raw_input PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_raw_input PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_gotoxy)
set_property(TARGET colmc_test_gotoxy PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_gotoxy PRIVATE src/test_gotoxy.cpp)
target_link_libraries(colmc_test_gotoxy colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_gotoxy PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_gotoxy PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_cursor_movement)
set_property(TARGET colmc_test_cursor_movement PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_cursor_movement PRIVATE src/test_cursor_movement.cpp)
target_link_libraries(colmc_test_cursor_movement colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_cursor_movement PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_cursor_movement PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_clear_screen)
set_property(TARGET colmc_test_clear_screen PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_clear_screen PRIVATE src/test_clear_screen.cpp)
target_link_libraries(colmc_test_clear_screen colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_clear_screen PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_clear_screen PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_clear_line)
set_property(TARGET colmc_test_clear_line PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_clear_line PRIVATE src/test_clear_line.cpp)
target_link_libraries(colmc_test_clear_line colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_clear_line PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_clear_line PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_get_term_size)
set_property(TARGET colmc_test_get_term_size PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_get_term_size PRIVATE src/test_get_term_size.cpp)
target_link_libraries(colmc_test_get_term_size colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_get_term_size PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_get_term_size PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_algorithms)
set_property(TARGET colmc_test_algorithms PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_algorithms PRIVATE src/colmc_test_algorithms.cpp)
target_link_libraries(colmc_test_algorithms colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_algorithms PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_algorithms PRIVATE -Wall -Wextra -Werror)
endif()

add_executable(colmc_test_styles)
set_property(TARGET colmc_test_styles PROPERTY POSITION_INDEPENDENT_CODE ON)
target_sources(colmc_test_styles PRIVATE src/colmc_test_styles.cpp)
target_link_libraries(colmc_test_styles colmc)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc_test_styles PRIVATE /W4 /WX)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc_test_styles PRIVATE -Wall -Wextra -Werror)
endif()
