using namespace std;

#include "TestHunspellStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/store/Directory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/Dictionary.h"
#include "../../../../../../java/org/apache/lucene/analysis/hunspell/HunspellStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"
#include "TestStemmer.h"

namespace org::apache::lucene::analysis::hunspell
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;
using Directory = org::apache::lucene::store::Directory;
using IOUtils = org::apache::lucene::util::IOUtils;
using org::junit::AfterClass;
using org::junit::BeforeClass;
shared_ptr<Dictionary> TestHunspellStemFilter::dictionary;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @BeforeClass public static void beforeClass() throws Exception
void TestHunspellStemFilter::beforeClass() 
{
  // no multiple try-with to workaround bogus VerifyError
  shared_ptr<InputStream> affixStream =
      TestStemmer::typeid->getResourceAsStream(L"simple.aff");
  shared_ptr<InputStream> dictStream =
      TestStemmer::typeid->getResourceAsStream(L"simple.dic");
  shared_ptr<Directory> tempDir = getDirectory();

  try {
    dictionary = make_shared<Dictionary>(tempDir, L"dictionary", affixStream,
                                         dictStream);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({affixStream, dictStream});
  }
  delete tempDir;
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @AfterClass public static void afterClass()
void TestHunspellStemFilter::afterClass() { dictionary.reset(); }

void TestHunspellStemFilter::testKeywordAttribute() 
{
  shared_ptr<MockTokenizer> tokenizer =
      whitespaceMockTokenizer(L"lucene is awesome");
  tokenizer->setEnableChecks(true);
  shared_ptr<HunspellStemFilter> filter =
      make_shared<HunspellStemFilter>(tokenizer, dictionary);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"lucene", L"lucen", L"is", L"awesome"},
      std::deque<int>{1, 0, 1, 1});

  // assert with keyword marker
  tokenizer = whitespaceMockTokenizer(L"lucene is awesome");
  shared_ptr<CharArraySet> set =
      make_shared<CharArraySet>(Arrays::asList(L"Lucene"), true);
  filter = make_shared<HunspellStemFilter>(
      make_shared<SetKeywordMarkerFilter>(tokenizer, set), dictionary);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"lucene", L"is", L"awesome"},
                            std::deque<int>{1, 1, 1});
}

void TestHunspellStemFilter::testLongestOnly() 
{
  shared_ptr<MockTokenizer> tokenizer =
      whitespaceMockTokenizer(L"lucene is awesome");
  tokenizer->setEnableChecks(true);
  shared_ptr<HunspellStemFilter> filter =
      make_shared<HunspellStemFilter>(tokenizer, dictionary, true, true);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"lucene", L"is", L"awesome"},
                            std::deque<int>{1, 1, 1});
}

void TestHunspellStemFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

TestHunspellStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestHunspellStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHunspellStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HunspellStemFilter>(tokenizer, dictionary));
}

void TestHunspellStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestHunspellStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestHunspellStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHunspellStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HunspellStemFilter>(tokenizer, dictionary));
}

void TestHunspellStemFilter::testIgnoreCaseNoSideEffects() 
{
  shared_ptr<Dictionary> *const d;
  // no multiple try-with to workaround bogus VerifyError
  shared_ptr<InputStream> affixStream =
      TestStemmer::typeid->getResourceAsStream(L"simple.aff");
  shared_ptr<InputStream> dictStream =
      TestStemmer::typeid->getResourceAsStream(L"simple.dic");
  shared_ptr<Directory> tempDir = getDirectory();
  try {
    d = make_shared<Dictionary>(tempDir, L"dictionary", affixStream,
                                Collections::singletonList(dictStream), true);
  }
  // C++ TODO: There is no native C++ equivalent to the exception 'finally'
  // clause:
  finally {
    IOUtils::closeWhileHandlingException({affixStream, dictStream});
  }
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this(), d);
  checkOneTerm(a, L"NoChAnGy", L"NoChAnGy");
  delete a;
  delete tempDir;
}

TestHunspellStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestHunspellStemFilter> outerInstance,
        shared_ptr<org::apache::lucene::analysis::hunspell::Dictionary> d)
{
  this->outerInstance = outerInstance;
  this->d = d;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestHunspellStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<HunspellStemFilter>(tokenizer, d));
}

shared_ptr<Directory> TestHunspellStemFilter::getDirectory()
{
  return newDirectory();
}
} // namespace org::apache::lucene::analysis::hunspell