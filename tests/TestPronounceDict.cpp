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
	PronounceDict m_dict;
};

TEST_F(TestPronounceDict, HashesInDictionary) {
	EXPECT_TRUE(m_dict.loadDictionary( PRONOUCE_DICT_HASHES ));

	EXPECT_TRUE(m_dict.contains("hello"));
	QList<QString> pronunciation_list = m_dict.getPronunciations("hello");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("hh ax l ow", pronunciation_list.value(0));

	EXPECT_TRUE(m_dict.contains("there"));
	pronunciation_list = m_dict.getPronunciations("there");
	EXPECT_EQ(2, pronunciation_list.size());
	EXPECT_EQ("dh ea", pronunciation_list.value(0));
	EXPECT_EQ("dh ea r", pronunciation_list.value(1));

	EXPECT_FALSE(m_dict.contains("goodbye"));
}

TEST_F(TestPronounceDict, SemicolonsInDictionary) {
	EXPECT_TRUE(m_dict.loadDictionary( PRONOUCE_DICT_SEMICOLON ));

	EXPECT_TRUE(m_dict.contains("hello"));
	QList<QString> pronunciation_list = m_dict.getPronunciations("hello");
	EXPECT_EQ(2, pronunciation_list.size());
	EXPECT_EQ("HH AH L OW", pronunciation_list.value(0));
	EXPECT_EQ("HH EH L OW", pronunciation_list.value(1));

	EXPECT_TRUE(m_dict.contains("there"));
	pronunciation_list = m_dict.getPronunciations("there");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("DH EH R", pronunciation_list.value(0));

	EXPECT_FALSE(m_dict.contains("goodbye"));
}

TEST_F(TestPronounceDict, LowercaseInDictionary ) {
	EXPECT_TRUE(m_dict.loadDictionary( PRONOUCE_DICT_LOWERCASE ));

	EXPECT_TRUE(m_dict.contains("hello"));
	QList<QString> pronunciation_list = m_dict.getPronunciations("hello");
	EXPECT_EQ(2, pronunciation_list.size());
	EXPECT_EQ("HH AH L OW", pronunciation_list.value(0));
	EXPECT_EQ("HH EH L OW", pronunciation_list.value(1));

	EXPECT_TRUE(m_dict.contains("there"));
	pronunciation_list = m_dict.getPronunciations("there");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("DH EH R", pronunciation_list.value(0));

	EXPECT_FALSE(m_dict.contains("goodbye"));
}

TEST_F(TestPronounceDict, HtkDictionaryFormat ) {
	EXPECT_TRUE(m_dict.loadDictionary( PRONOUCE_DICT_HTK ));

	EXPECT_TRUE(m_dict.contains("abandon"));
	QList<QString> pronunciation_list = m_dict.getPronunciations("abandon");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("ax b ae n d ax n", pronunciation_list.value(0));

	EXPECT_TRUE(m_dict.contains("abandoned"));
	pronunciation_list = m_dict.getPronunciations("abandoned");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("ax b ae n d ax n d", pronunciation_list.value(0));

	EXPECT_TRUE(m_dict.contains("abandonment"));
	pronunciation_list = m_dict.getPronunciations("abandonment");
	EXPECT_EQ(1, pronunciation_list.size());
	EXPECT_EQ("ax b ae n d ax n m ax n t", pronunciation_list.value(0));

	EXPECT_FALSE(m_dict.contains("zoom"));
	pronunciation_list = m_dict.getPronunciations("zoom");
	EXPECT_EQ(0, pronunciation_list.size());
}

} // namespace
