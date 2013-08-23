/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <config.h>
#include <VoiceService.h>
#include <Localisation.h>
#include <common/DBusTypes.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <csignal>

static void exitQt(int sig) {
	Q_UNUSED(sig);
	QCoreApplication::exit(0);
}

int main(int argc, char *argv[]) {
	QCoreApplication application(argc, argv);
	DBusTypes::registerMetaTypes();

	setlocale(LC_ALL, "");
	bindtextdomain(GETTEXT_PACKAGE, LOCALE_DIR);
	textdomain(GETTEXT_PACKAGE);

	signal(SIGINT, &exitQt);
	signal(SIGTERM, &exitQt);

	QDBusConnection connection = QDBusConnection::sessionBus();
	connection.registerService("com.canonical.Unity.Voice");

	QString deviceName("");
	if (argc == 2) {
		deviceName = QString::fromUtf8(argv[1]);
	}

	VoiceService voiceService(QDBusConnection::sessionBus(), deviceName);

	int result = application.exec();
	connection.unregisterService("com.canonical.Unity.Voice");

	return result;
}
