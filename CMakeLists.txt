project(unity-voice CXX)
cmake_minimum_required(VERSION 2.8.9)

set(GETTEXT_PACKAGE unity-voice)

set(API_VERSION 1)
set(ABI_VERSION 1)

set(HMM_PATH "/usr/share/sphinx-voxforge-en/hmm/voxforge_en_sphinx.cd_cont_3000/" CACHE PATH "Path to hidden markov model")
set(DICT_PATH "/usr/share/sphinx-voxforge-en/lm/voxforge_en_sphinx.cd_cont_3000/voxforge_en_sphinx.dic" CACHE PATH "Path to dictionary")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

option(ENABLE_VOICE_TESTS "Determine if voice tests will be run" OFF)

find_package(PkgConfig REQUIRED)

find_package(Qt5Core REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})

find_package(Qt5DBus COMPONENTS Qt5DBusMacros REQUIRED)
include_directories(${Qt5DBus_INCLUDE_DIRS})

pkg_check_modules(POCKETSPHINX REQUIRED pocketsphinx)
include_directories(${POCKETSPHINX_INCLUDE_DIRS})

find_package(Qt5Concurrent REQUIRED)
include_directories(${Qt5Concurrent_INCLUDE_DIRS})

find_package(Qt5Test REQUIRED)
include_directories(${Qt5Test_INCLUDE_DIRS})

pkg_check_modules(QTDBUSTEST REQUIRED libqtdbustest-1 REQUIRED)
include_directories(${QTDBUSTEST_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DATA_DIR "${CMAKE_SOURCE_DIR}/data")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")
set(SOURCE_BINARY_DIR "${CMAKE_BINARY_DIR}/src")
include_directories(${SOURCE_DIR})
include_directories(${SOURCE_BINARY_DIR})

include(GNUInstallDirs)
include(Coverage)

include(GenerateExportHeader)
add_compiler_export_flags()

# Workaround for libexecdir on debian
if (EXISTS "/etc/debian_version") 
  set(CMAKE_INSTALL_LIBEXECDIR ${CMAKE_INSTALL_LIBDIR})
  set(CMAKE_INSTALL_FULL_LIBEXECDIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}")
endif()

set(LOCALE_DIR "${CMAKE_INSTALL_FULL_DATADIR}/locale")

add_definitions(
	-std=c++11
	-fno-permissive
	-pedantic
	-Wall
	-Wextra
)

configure_file(
	"${SOURCE_DIR}/config.h.in"
	"${SOURCE_BINARY_DIR}/config.h"
)

add_subdirectory("src")
add_subdirectory("data")

enable_testing()
ADD_CUSTOM_TARGET(
	check
	${CMAKE_CTEST_COMMAND} --force-new-ctest-process --output-on-failure
)

add_subdirectory(tests)


