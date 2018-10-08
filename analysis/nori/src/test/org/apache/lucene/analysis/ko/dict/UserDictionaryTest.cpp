using namespace std;

#include "UserDictionaryTest.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/POS.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/Dictionary.h"
#include "../../../../../../../java/org/apache/lucene/analysis/ko/dict/UserDictionary.h"
#include "../TestKoreanTokenizer.h"

namespace org::apache::lucene::analysis::ko::dict
{
using POS = org::apache::lucene::analysis::ko::POS;
using TestKoreanTokenizer =
    org::apache::lucene::analysis::ko::TestKoreanTokenizer;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testLookup() throws java.io.IOException
void UserDictionaryTest::testLookup() 
{
  shared_ptr<UserDictionary> dictionary = TestKoreanTokenizer::readDict();
  wstring s = L"세종";
  std::deque<wchar_t> sArray = s.toCharArray();
  deque<int> wordIds = dictionary->lookup(sArray, 0, s.length());
  assertEquals(1, wordIds.size());
  assertNull(dictionary->getMorphemes(wordIds[0], sArray, 0, s.length()));

  s = L"세종시";
  sArray = s.toCharArray();
  wordIds = dictionary->lookup(sArray, 0, s.length());
  assertEquals(2, wordIds.size());
  assertNull(dictionary->getMorphemes(wordIds[0], sArray, 0, s.length()));

  std::deque<std::shared_ptr<Dictionary::Morpheme>> decompound =
      dictionary->getMorphemes(wordIds[1], sArray, 0, s.length());
  assertTrue(decompound.size() == 2);
  assertEquals(decompound[0]->posTag, POS::Tag::NNG);
  assertEquals(decompound[0]->surfaceForm, L"세종");
  assertEquals(decompound[1]->posTag, POS::Tag::NNG);
  assertEquals(decompound[1]->surfaceForm, L"시");

  s = L"c++";
  sArray = s.toCharArray();
  wordIds = dictionary->lookup(sArray, 0, s.length());
  assertEquals(1, wordIds.size());
  assertNull(dictionary->getMorphemes(wordIds[0], sArray, 0, s.length()));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testRead() throws java.io.IOException
void UserDictionaryTest::testRead() 
{
  shared_ptr<UserDictionary> dictionary = TestKoreanTokenizer::readDict();
  assertNotNull(dictionary);
}
} // namespace org::apache::lucene::analysis::ko::dict