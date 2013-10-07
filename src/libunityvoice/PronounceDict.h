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
 * Author: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#ifndef PRONOUNCEDICT_H_
#define PRONOUNCEDICT_H_

#include <QHash>
#include <QString>

class Q_DECL_EXPORT PronounceDict final {
public:
	Q_DECL_EXPORT
	bool loadDictionary(const QString& dictPath );

	Q_DECL_EXPORT
	bool contains( const QString& word );

	Q_DECL_EXPORT
	QList<QString> getPronunciations( const QString& word );

private:
	QHash< QString, QList<QString> > m_dict;
};

#endif /* PRONOUNCEDICT_H_ */
