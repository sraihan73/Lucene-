using namespace std;

#include "UserDictionaryTest.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ja/dict/UserDictionary.h"
#include "../TestJapaneseTokenizer.h"

namespace org::apache::lucene::analysis::ja::dict
{
using TestJapaneseTokenizer =
    org::apache::lucene::analysis::ja::TestJapaneseTokenizer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLookup() throws java.io.IOException
void UserDictionaryTest::testLookup() 
{
  shared_ptr<UserDictionary> dictionary = TestJapaneseTokenizer::readDict();
  wstring s = L"関西国際空港に行った";
  std::deque<std::deque<int>> dictionaryEntryResult =
      dictionary->lookup(s.toCharArray(), 0, s.length());
  // Length should be three 関西, 国際, 空港
  assertEquals(3, dictionaryEntryResult.size());

  // Test positions
  assertEquals(0, dictionaryEntryResult[0][1]); // index of 関西
  assertEquals(2, dictionaryEntryResult[1][1]); // index of 国際
  assertEquals(4, dictionaryEntryResult[2][1]); // index of 空港

  // Test lengths
  assertEquals(2, dictionaryEntryResult[0][2]); // length of 関西
  assertEquals(2, dictionaryEntryResult[1][2]); // length of 国際
  assertEquals(2, dictionaryEntryResult[2][2]); // length of 空港

  s = L"関西国際空港と関西国際空港に行った";
  std::deque<std::deque<int>> dictionaryEntryResult2 =
      dictionary->lookup(s.toCharArray(), 0, s.length());
  // Length should be six
  assertEquals(6, dictionaryEntryResult2.size());
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testReadings() throws java.io.IOException
void UserDictionaryTest::testReadings() 
{
  shared_ptr<UserDictionary> dictionary = TestJapaneseTokenizer::readDict();
  std::deque<std::deque<int>> result =
      dictionary->lookup((wstring(L"日本経済新聞")).toCharArray(), 0, 6);
  assertEquals(3, result.size());
  int wordIdNihon = result[0][0]; // wordId of 日本 in 日本経済新聞
  assertEquals(L"ニホン",
               dictionary->getReading(wordIdNihon,
                                      (wstring(L"日本")).toCharArray(), 0, 2));

  result = dictionary->lookup((wstring(L"朝青龍")).toCharArray(), 0, 3);
  assertEquals(1, result.size());
  int wordIdAsashoryu = result[0][0]; // wordId for 朝青龍
  assertEquals(L"アサショウリュウ",
               dictionary->getReading(
                   wordIdAsashoryu, (wstring(L"朝青龍")).toCharArray(), 0, 3));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPartOfSpeech() throws
// java.io.IOException
void UserDictionaryTest::testPartOfSpeech() 
{
  shared_ptr<UserDictionary> dictionary = TestJapaneseTokenizer::readDict();
  std::deque<std::deque<int>> result =
      dictionary->lookup((wstring(L"日本経済新聞")).toCharArray(), 0, 6);
  assertEquals(3, result.size());
  int wordIdKeizai = result[1][0]; // wordId of 経済 in 日本経済新聞
  assertEquals(L"カスタム名詞", dictionary->getPartOfSpeech(wordIdKeizai));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRead() throws java.io.IOException
void UserDictionaryTest::testRead() 
{
  shared_ptr<UserDictionary> dictionary = TestJapaneseTokenizer::readDict();
  assertNotNull(dictionary);
}
} // namespace org::apache::lucene::analysis::ja::dict