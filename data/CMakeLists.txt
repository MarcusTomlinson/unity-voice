
###########################
# Upstart
###########################

configure_file (
	unity-voice.conf.in
	unity-voice.conf
	@ONLY
)

install (
	FILES ${CMAKE_CURRENT_BINARY_DIR}/unity-voice.conf
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/upstart/sessions
)

###########################
# DBus Interface
###########################

install(
	FILES
	com.canonical.Unity.Voice.xml
	DESTINATION "${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces/"
)

###########################
# DBus Service
###########################

set(VOICE_SERVICE
	"${CMAKE_CURRENT_BINARY_DIR}/com.canonical.Unity.Voice.service"
)

configure_file(
	"com.canonical.Unity.Voice.service.in"
	${VOICE_SERVICE}
	@ONLY
)

install(
	FILES
	${VOICE_SERVICE}
	DESTINATION "${CMAKE_INSTALL_DATADIR}/dbus-1/services/"
)
