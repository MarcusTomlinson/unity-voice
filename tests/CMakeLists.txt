
set(CMAKE_AUTOMOC OFF)
include(FindGMock)
set(CMAKE_AUTOMOC ON)

find_package("Valgrind" REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${GMOCK_INCLUDE_DIRS})
include_directories(${GTEST_INCLUDE_DIRS})

add_definitions(-DSOUNDS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data/sounds")
add_definitions(-DUNITY_VOICE_SERVICE_BIN="${CMAKE_BINARY_DIR}/src/service/unity-voice-service")

add_definitions(-DPRONOUCE_DICT_HASHES="${CMAKE_CURRENT_SOURCE_DIR}/data/test-pronounce-dict-hashes.dic")
add_definitions(-DPRONOUCE_DICT_SEMICOLON="${CMAKE_CURRENT_SOURCE_DIR}/data/test-pronounce-dict-semicolon.dic")
add_definitions(-DPRONOUCE_DICT_LOWERCASE="${CMAKE_CURRENT_SOURCE_DIR}/data/test-pronounce-dict-lowercase.dic")
add_definitions(-DPRONOUCE_DICT_HTK="${CMAKE_CURRENT_SOURCE_DIR}/data/test-pronounce-dict-htk.dic")

set(
	UNIT_TESTS_SRC
        TestPronounceDict.cpp
	TestVoiceService.cpp
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
	${QTDBUSTEST_LIBRARIES}
	${GTEST_LIBRARIES}
)

add_valgrind_test(
	unit-tests
	unit-tests
)

# Install autopilot tests
set(AUTOPILOT_DIR autopilot/unity_voice)

execute_process(COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
    OUTPUT_VARIABLE PYTHON_PACKAGE_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

install(DIRECTORY ${AUTOPILOT_DIR}
    DESTINATION ${PYTHON_PACKAGE_DIR}
    )
