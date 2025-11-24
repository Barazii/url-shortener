cmake_minimum_required(VERSION 3.16)
project(url_shortener LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)

find_package(SQLite3 QUIET)
if(NOT SQLite3_FOUND)
	find_path(SQLITE3_INCLUDE_DIR sqlite3.h)
	find_library(SQLITE3_LIBRARY sqlite3)
	if(NOT SQLITE3_INCLUDE_DIR OR NOT SQLITE3_LIBRARY)
		message(FATAL_ERROR "sqlite3 development files not found. Install libsqlite3-dev or equivalent.")
	endif()
	add_library(SQLite3::SQLite3 UNKNOWN IMPORTED)
	set_target_properties(SQLite3::SQLite3 PROPERTIES
		IMPORTED_LOCATION ${SQLITE3_LIBRARY}
		INTERFACE_INCLUDE_DIRECTORIES ${SQLITE3_INCLUDE_DIR}
	)
endif()

add_library(shortener STATIC src/Shortener.cpp)
target_include_directories(shortener PUBLIC include)
target_link_libraries(shortener PUBLIC SQLite::SQLite3)

add_executable(url_shortener src/main.cpp)
target_link_libraries(url_shortener PRIVATE shortener)

enable_testing()
add_executable(shortener_tests tests/shortener_tests.cpp)
target_link_libraries(shortener_tests PRIVATE shortener)
add_test(NAME ShortenerBasic COMMAND shortener_tests)
