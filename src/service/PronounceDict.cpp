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

bool PronounceDict::loadDictionary( const QString& dictPath )
{
	QFile file( dictPath );

	// attempt to open dictionary file
	if( !file.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		return false;
	}

	// reset dictionary
	m_dict.clear();

	QTextStream in( &file );
	while( !in.atEnd() )
	{
		// read next line
		QString line = in.readLine();

		// ignore line if empty or begins with ";;;" / "#"
		if(
			line == "" ||
			line.indexOf( ";;;", 0 ) == 0 ||
			line.indexOf( '#', 0 ) == 0 )
		{
			continue;
		}

		// word ends at either first occurrence of tab, or '(' from 2nd char
		int word_end_pos = 0;
		int tab_pos = line.indexOf( '\t', 0 );
		int bracket_pos = line.indexOf( '(', 1 );

		if( bracket_pos != -1 && bracket_pos < tab_pos )
		{
			word_end_pos = bracket_pos;
		}
		else
		{
			word_end_pos = tab_pos;
		}

		QString word = line.left( word_end_pos ).toLower();

		// pronunciation starts just after last occurrence of a tab
		int pronounce_start_pos = 0;
		int next_tab_pos = 0;

		while( next_tab_pos != -1 )
		{
			next_tab_pos = line.indexOf( '\t', next_tab_pos + 1 );

			if( next_tab_pos != -1 )
			{
				pronounce_start_pos = next_tab_pos;
			}
		}

		QString pronunciation = line.right( pronounce_start_pos );

		// insert pronunciation into dictionary
		QList<QString> pronunciation_list;

		if( m_dict.contains( word ) )
		{
			pronunciation_list = m_dict.value( word );
		}

		pronunciation_list.append( pronunciation );
		m_dict.insert( word, pronunciation_list );
	}

	file.close();

	return true;
}

bool PronounceDict::contains( const QString& word )
{
	return m_dict.contains( word.toLower() );
}

QList<QString> PronounceDict::getPronunciations( const QString& word )
{
	return m_dict.value( word.toLower() );
}
