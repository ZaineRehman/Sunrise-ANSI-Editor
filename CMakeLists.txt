cmake_minimum_required(VERSION 3.23)
project(Sunrise_ANSI_Editor LANGUAGES CXX)

# version
set(PROJECT_VERSION "0.1.0-alpha")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

set_property(DIRECTORY PROPERTY DESCRIPTION "A terminal-based ANSI/ASCII-art editor")
set_property(DIRECTORY PROPERTY HOMEPAGE_URL "https://github.com/ZaineRehman/Sunrise-ANSI-Editor")


# set output directory for build objects
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# sources
add_executable(Sunrise_ANSI_Editor 
	${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/input.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/output.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/art.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ANSI.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/precalculate.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/lib.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/file_io.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/encodings.cpp
)

target_include_directories(Sunrise_ANSI_Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(
	TARGETS Sunrise_ANSI_Editor 
	RUNTIME_DEPENDENCY_SET program_deps
	DESTINATION bin
)


# packaging

set(CPACK_PACKAGE_NAME "Sunrise ANSI Editor")
set(CPACK_PACKAGE_VENDOR "Zaine Rehman")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "ANSI art editor")

if(WIN32)
	set(CPACK_GENERATOR ZIP)
else()
	set(CPACK_GENERATOR TGZ)
endif()

include(CPack)