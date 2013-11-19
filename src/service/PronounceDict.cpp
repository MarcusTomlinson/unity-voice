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

#include <PronounceDict.h>

#include <QFile>
#include <QTextStream>

bool PronounceDict::loadDictionary(const QString& dictPath) {
	QFile file(dictPath);

	// attempt to open dictionary file
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return false;
	}

	// reset dictionary
	m_dict.clear();

	QTextStream in(&file);
	while (!in.atEnd()) {
		//-- read next line
		QString line = in.readLine();

		// ignore line if empty or begins with ";;;" / "#"
		if (line == "" || line.indexOf(";;;", 0) == 0
				|| line.indexOf('#', 0) == 0) {
			continue;
		}

		//-- word ends at first occurrence of tab, space, or '(' from 2nd char
		int word_end_pos = 1;

		for (; word_end_pos < line.size(); ++word_end_pos) {
			if (line[word_end_pos] == '\t' || line[word_end_pos] == ' '
					|| line[word_end_pos] == '(') {
				break;
			}
		}

		QString word = line.left(word_end_pos).toLower();

		//-- pronunciation starts just after last occurrence of a coninuous tabs / spaces
		int pronounce_start_pos = 0;

		// if we are reading a htk dictionary, skip to after [...] section
		if (line.indexOf('[', 0) > 0) {
			pronounce_start_pos = line.indexOf(']', 0) + 1;
		}
		// if we're on a duplicate word line, skip to after ')'
		else if (line[word_end_pos] == '(') {
			pronounce_start_pos = line.indexOf(')', word_end_pos) + 1;
		} else {
			pronounce_start_pos = word_end_pos;
		}

		// find first non-space, non-tab
		for (; pronounce_start_pos < line.size(); ++pronounce_start_pos) {
			if (line[pronounce_start_pos] != '\t'
					&& line[pronounce_start_pos] != ' ') {
				break;
			}
		}

		// construct pronunciation (skipping numeric chars)
		QString pronunciation;
		for (; pronounce_start_pos < line.size(); ++pronounce_start_pos) {
			if (!line[pronounce_start_pos].isDigit()) {
				pronunciation.push_back(line[pronounce_start_pos]);
			}
		}

		//-- insert pronunciation into dictionary
		QList<QString> pronunciation_list;

		if (m_dict.contains(word)) {
			pronunciation_list = m_dict.value(word);
		}

		pronunciation_list.append(pronunciation);
		m_dict.insert(word, pronunciation_list);
	}

	// close dictionary file
	file.close();

	return true;
}

bool PronounceDict::contains(const QString& word) {
	return m_dict.contains(word.toLower());
}

QList<QString> PronounceDict::getPronunciations(const QString& word) {
	return m_dict.value(word.toLower());
}
