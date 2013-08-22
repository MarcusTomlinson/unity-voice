/*
 * UnityVoice.h
 *
 *  Created on: 22 Aug 2013
 *      Author: pete
 */

#ifndef UNITYVOICE_H_
#define UNITYVOICE_H_

#include <libunityvoice/VoiceInterface.h>

namespace LibUnityVoice {

class Q_DECL_EXPORT UnityVoice {
public:
	Q_DECL_EXPORT
	static ComCanonicalUnityVoiceInterface* getInstance();
};

} /* namespace LibUnityVoice */
#endif /* UNITYVOICE_H_ */
