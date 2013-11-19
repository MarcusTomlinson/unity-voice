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

#ifndef VOICESERVICE_H_
#define VOICESERVICE_H_

#include <service/PronounceDict.h>

#include <QList>
#include <QString>
#include <QScopedPointer>
#include <QDBusConnection>
#include <QtDBus>

#include <pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/cont_ad.h>

class VoiceAdaptor;

class VoiceService: public QObject, protected QDBusContext {
Q_OBJECT
public:
	explicit VoiceService(const QDBusConnection &connection,
			const QString &deviceName, QObject *parent = 0);

	virtual ~VoiceService();

public Q_SLOTS:
	virtual QString listen(const QList<QStringList> &commands);

protected:
	QString utteranceLoop();

	QString sphinxListen(fsg_model_t* fsg);

	static int calculateNumberOfStates(const QList<QStringList> &commands);

	int writeCommand(fsg_model_t *fsg, const QStringList &command, int stateNum,
			float commandProbability);

	fsg_model_t* buildGrammar(const QList<QStringList> &commands);

	static arg_t sphinx_cmd_ln[];

	QScopedPointer<VoiceAdaptor> m_adaptor;

	QDBusConnection m_connection;

	cmd_ln_t *config;

	ps_decoder_t *ps;

	PronounceDict m_dict;
};

#endif /* VOICESERVICE_H_ */
