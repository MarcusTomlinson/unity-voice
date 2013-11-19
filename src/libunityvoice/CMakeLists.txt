
set(
	LIB_UNITY_VOICE_PUBLIC_HEADERS
	UnityVoice.h
	"${CMAKE_CURRENT_BINARY_DIR}/VoiceInterface.h"
)

set(
	LIB_UNITY_VOICE_SOURCES
	UnityVoice.cpp
)

set_source_files_properties(
	"${DATA_DIR}/com.canonical.Unity.Voice.xml"
	PROPERTIES
	INCLUDE "libunityvoice/UnityVoice.h"
	NO_NAMESPACE YES
)

qt5_add_dbus_interface(
	LIB_UNITY_VOICE_SOURCES
	"${DATA_DIR}/com.canonical.Unity.Voice.xml"
	VoiceInterface
)

add_library(
	unityvoice
	SHARED
	${LIB_UNITY_VOICE_SOURCES}
)

qt5_use_modules(
	unityvoice
	Core
	DBus
)

set_target_properties(
	unityvoice
	PROPERTIES
	VERSION ${API_VERSION}.0.0
	SOVERSION ${ABI_VERSION}
	PUBLIC_HEADER "${LIB_UNITY_VOICE_PUBLIC_HEADERS}"
)

install(
  TARGETS unityvoice
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libunityvoice-${API_VERSION}/libunityvoice"
)

# Package config

set(LIB_UNITY_VOICE_PC "libunityvoice-${API_VERSION}.pc")
configure_file("libunityvoice.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/${LIB_UNITY_VOICE_PC}" @ONLY)

install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${LIB_UNITY_VOICE_PC}"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

