using namespace std;

#include "TestWordlistLoader.h"

namespace org::apache::lucene::analysis
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using WordlistLoader = org::apache::lucene::analysis::WordlistLoader;

void TestWordlistLoader::testWordlistLoading() 
{
  wstring s = L"ONE\n  two \nthree";
  shared_ptr<CharArraySet> wordSet1 =
      WordlistLoader::getWordSet(make_shared<StringReader>(s));
  checkSet(wordSet1);
  shared_ptr<CharArraySet> wordSet2 = WordlistLoader::getWordSet(
      make_shared<BufferedReader>(make_shared<StringReader>(s)));
  checkSet(wordSet2);
}

void TestWordlistLoader::testComments() 
{
  wstring s = L"ONE\n  two \nthree\n#comment";
  shared_ptr<CharArraySet> wordSet1 =
      WordlistLoader::getWordSet(make_shared<StringReader>(s), L"#");
  checkSet(wordSet1);
  assertFalse(wordSet1->contains(L"#comment"));
  assertFalse(wordSet1->contains(L"comment"));
}

void TestWordlistLoader::checkSet(shared_ptr<CharArraySet> wordset)
{
  assertEquals(3, wordset->size());
  assertTrue(wordset->contains(L"ONE")); // case is not modified
  assertTrue(wordset->contains(L"two")); // surrounding whitespace is removed
  assertTrue(wordset->contains(L"three"));
  assertFalse(wordset->contains(L"four"));
}

void TestWordlistLoader::testSnowballListLoading() 
{
  wstring s = wstring(L"|comment\n") + // commented line
              L" |comment\n" +         // commented line with leading whitespace
              L"\n" +                  // blank line
              L"  \t\n" +              // line with only whitespace
              L" |comment | comment\n" + // commented line with comment
              L"ONE\n" +                 // stopword, in uppercase
              L"   two   \n" +           // stopword with leading/trailing space
              L" three   four five \n" + // multiple stopwords
              L"six seven | comment\n";  // multiple stopwords + comment
  shared_ptr<CharArraySet> wordset =
      WordlistLoader::getSnowballWordSet(make_shared<StringReader>(s));
  assertEquals(7, wordset->size());
  assertTrue(wordset->contains(L"ONE"));
  assertTrue(wordset->contains(L"two"));
  assertTrue(wordset->contains(L"three"));
  assertTrue(wordset->contains(L"four"));
  assertTrue(wordset->contains(L"five"));
  assertTrue(wordset->contains(L"six"));
  assertTrue(wordset->contains(L"seven"));
}
} // namespace org::apache::lucene::analysis