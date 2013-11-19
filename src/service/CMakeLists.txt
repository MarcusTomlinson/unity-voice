
#########################################
# Service library
#########################################

set(
	UNITY_VOICE_SOURCES
	PronounceDict.cpp
	VoiceService.cpp
	main.cpp
)

qt5_add_dbus_adaptor(
	UNITY_VOICE_SOURCES
	"${DATA_DIR}/com.canonical.Unity.Voice.xml"
	AdaptorInclude.h
	VoiceService
	VoiceAdaptor
)

add_library(
	unity-voice-service
	STATIC
	${UNITY_VOICE_SOURCES}
)

qt5_use_modules(
	unity-voice-service
	Core
	DBus
)

target_link_libraries(
	unity-voice-service
	${POCKETSPHINX_LIBRARIES}
)

#########################################
# Service binary
#########################################

add_executable(
	unity-voice-service-bin
	${UNITY_VOICE_SOURCES}
)

set_target_properties(
	unity-voice-service-bin
	PROPERTIES
	OUTPUT_NAME "unity-voice-service"
)

target_link_libraries(
	unity-voice-service-bin
	unity-voice-service
)

qt5_use_modules(
	unity-voice-service-bin
	Core
)

install(
	TARGETS unity-voice-service-bin
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
)
