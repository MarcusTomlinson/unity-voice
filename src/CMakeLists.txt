
set(
	UNITY_VOICE_SOURCES
	#PronounceDict.cpp
	VoiceService.cpp
)

qt5_add_dbus_adaptor(
	UNITY_VOICE_SOURCES
	"${DATA_DIR}/com.canonical.Unity.Voice.xml"
	AdaptorInclude.h
	VoiceService
	VoiceAdaptor
)

add_library(
	unity-voice
	STATIC
	${UNITY_VOICE_SOURCES}
)

qt5_use_modules(
	unity-voice
	Core
	DBus
)

target_link_libraries(
	unity-voice
	${POCKETSPHINX_LIBRARIES}
)

add_executable(
	unity-voice-service
	main.cpp
)

qt5_use_modules(
	unity-voice-service
	Core
)

target_link_libraries(
	unity-voice-service
	unity-voice
)

install(
	TARGETS unity-voice-service
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
)
