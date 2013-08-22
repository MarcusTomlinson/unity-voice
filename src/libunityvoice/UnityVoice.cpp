/*
 * UnityVoice.cpp
 *
 *  Created on: 22 Aug 2013
 *      Author: pete
 */

#include <libunityvoice/UnityVoice.h>

namespace LibUnityVoice {

ComCanonicalUnityVoiceInterface * UnityVoice::getInstance() {
	return new ComCanonicalUnityVoiceInterface("com.canonical.Unity.Voice",
			"/com/canonical/Unity/Voice", QDBusConnection::sessionBus());
}

} /* namespace LibUnityVoice */
