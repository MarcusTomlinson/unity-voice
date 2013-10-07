
set(
        UNITY_VOICE_SOURCES
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

add_executable(
	unity-voice-service
	${UNITY_VOICE_SOURCES}
)

qt5_use_modules(
	unity-voice-service
	Core
	DBus
)

target_link_libraries(
	unity-voice-service
        unityvoice
	${POCKETSPHINX_LIBRARIES}
)

install(
	TARGETS unity-voice-service
	RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
)
