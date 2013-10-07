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

#include <libunityvoice/PronounceDict.h>

#include <gtest/gtest.h>
#include <iostream>

using namespace testing;

namespace {

class TestPronounceDict: public Test {
protected:
	void testHashes() {
		ASSERT_TRUE( m_dict.loadDictionary( PRONOUCE_DICT_HASHES ) );

		QList<QString> pronunciation_list = m_dict.getPronunciations( "hello" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "hh ax l ow" );

		pronunciation_list = m_dict.getPronunciations( "there" );
		ASSERT_TRUE( pronunciation_list.size() == 2 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "dh ea" );
		ASSERT_TRUE( pronunciation_list.value( 1 ) == "dh ea r" );
	}

	void testSemicolon() {
		ASSERT_TRUE( m_dict.loadDictionary( PRONOUCE_DICT_SEMICOLON ) );

		QList<QString> pronunciation_list = m_dict.getPronunciations( "hello" );
		ASSERT_TRUE( pronunciation_list.size() == 2 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "HH AH L OW" );
		ASSERT_TRUE( pronunciation_list.value( 1 ) == "HH EH L OW" );

		pronunciation_list = m_dict.getPronunciations( "there" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "DH EH R" );
	}

	void testLowercase() {
		ASSERT_TRUE( m_dict.loadDictionary( PRONOUCE_DICT_LOWERCASE ) );

		QList<QString> pronunciation_list = m_dict.getPronunciations( "hello" );
		ASSERT_TRUE( pronunciation_list.size() == 2 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "HH AH L OW" );
		ASSERT_TRUE( pronunciation_list.value( 1 ) == "HH EH L OW" );

		pronunciation_list = m_dict.getPronunciations( "there" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "DH EH R" );
	}

	void testHtk() {
		ASSERT_TRUE( m_dict.loadDictionary( PRONOUCE_DICT_HTK ) );

		QList<QString> pronunciation_list = m_dict.getPronunciations( "abandon" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "ax b ae n d ax n" );

		pronunciation_list = m_dict.getPronunciations( "abandoned" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "ax b ae n d ax n d" );

		pronunciation_list = m_dict.getPronunciations( "abandonment" );
		ASSERT_TRUE( pronunciation_list.size() == 1 );
		ASSERT_TRUE( pronunciation_list.value( 0 ) == "ax b ae n d ax n m ax n t" );

		pronunciation_list = m_dict.getPronunciations( "zoom" );
		ASSERT_TRUE( pronunciation_list.size() == 0 );
	}

	PronounceDict m_dict;
};

TEST_F(TestPronounceDict, DictionaryTypes) {
	testHashes();
	testSemicolon();
	testLowercase();
	testHtk();
}

} // namespace
