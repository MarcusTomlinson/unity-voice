/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#ifndef UNITYVOICE_H_
#define UNITYVOICE_H_

#include <QStringList>

class ComCanonicalUnityVoiceInterface;

typedef QList<QStringList> QListStringList;

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
