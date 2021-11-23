project(ColorMyConsole)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_library(colmc STATIC)
set_property(TARGET colmc PROPERTY POSITION_INDEPENDENT_CODE ON)

set(SOURCES
	include/colmc/push_warnings.h
	include/colmc/pop_warnings.h
	include/colmc/colmc.h
	include/colmc/raw_input.h
	include/colmc/sequences.h
	include/colmc/setup.h
	include/colmc/term_size.h
	src/colmc/algorithms.h
	src/colmc/posix/setup.cpp
	src/colmc/windows/setup.cpp
)

target_sources(colmc PRIVATE ${SOURCES})
target_include_directories(colmc PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(colmc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_definitions(colmc PRIVATE colmc_BUILD)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(colmc PRIVATE /W4 /WX)
  target_compile_definitions(colmc_test PRIVATE _SCL_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(colmc PRIVATE -Wall -Wextra -Werror)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})

add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
