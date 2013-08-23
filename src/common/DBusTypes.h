/*
 * DBusTypes.h
 *
 *  Created on: 22 Aug 2013
 *      Author: pete
 */

#ifndef DBUSTYPES_H_
#define DBUSTYPES_H_

#include <QStringList>
#include <QtDBus>

typedef QList<QStringList> QListStringList;

class DBusTypes {
public:
	static void registerMetaTypes() {
		qRegisterMetaType<QListStringList>("QListStringList");
		qDBusRegisterMetaType<QListStringList>();
	}
};

#endif /* DBUSTYPES_H_ */
