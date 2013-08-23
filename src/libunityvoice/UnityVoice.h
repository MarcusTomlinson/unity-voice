/*
 * UnityVoice.h
 *
 *  Created on: 22 Aug 2013
 *      Author: pete
 */

#ifndef UNITYVOICE_H_
#define UNITYVOICE_H_

#include <QStringList>

class ComCanonicalUnityVoiceInterface;

typedef QList<QStringList> QListStringList;
//Q_DECLARE_METATYPE(QListStringList)

namespace LibUnityVoice {

class Q_DECL_EXPORT UnityVoice {
public:
	Q_DECL_EXPORT
	static ComCanonicalUnityVoiceInterface* getInstance();

	Q_DECL_EXPORT
	static void registerMetaTypes();
};

} /* namespace LibUnityVoice */
#endif /* UNITYVOICE_H_ */
