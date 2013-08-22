
set(CMAKE_AUTOMOC OFF)
include(FindGMock)
set(CMAKE_AUTOMOC ON)

find_package("Valgrind" REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_definitions(-DSOUNDS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/sounds")

add_definitions(-DPRONOUCE_DICT_HASHES="${CMAKE_CURRENT_SOURCE_DIR}/data/dicts/hashes.dic")
add_definitions(-DPRONOUCE_DICT_HTK="${CMAKE_CURRENT_SOURCE_DIR}/data/dicts/htk.dic")
add_definitions(-DPRONOUCE_DICT_LOWERCASE="${CMAKE_CURRENT_SOURCE_DIR}/data/dicts/lowercase.dic")
add_definitions(-DPRONOUCE_DICT_SEMICOLON="${CMAKE_CURRENT_SOURCE_DIR}/data/dicts/semicolon.dic")

add_definitions(-DUNITY_VOICE_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/service/unity-voice-service")

set(
	UNIT_TESTS_SRC
	TestSphinx.cpp
	main.cpp
)

add_executable(
	unit-tests
	${UNIT_TESTS_SRC}
)

qt5_use_modules(
	unit-tests
	Core
	DBus
	Test
)

target_link_libraries(
	unit-tests
	unityvoice
	${POCKETSPHINX_LIBRARIES}
	${QTDBUSMOCK_LIBRARIES}
	${QTDBUSTEST_LIBRARIES}
	${GTEST_LIBRARIES}
	${GMOCK_LIBRARIES}
)

add_valgrind_test(
	unit-tests
	unit-tests
)
