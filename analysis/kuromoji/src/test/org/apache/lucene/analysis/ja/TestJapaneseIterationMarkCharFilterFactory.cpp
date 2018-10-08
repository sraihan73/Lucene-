using namespace std;

#include "TestJapaneseIterationMarkCharFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseIterationMarkCharFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharFilter = org::apache::lucene::analysis::CharFilter;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseIterationMarkCharFilterFactory::
    testIterationMarksWithKeywordTokenizer() 
{
  const wstring text = L"時々馬鹿々々しいところゞゝゝミスヾ";
  shared_ptr<JapaneseIterationMarkCharFilterFactory> filterFactory =
      make_shared<JapaneseIterationMarkCharFilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<CharFilter> filter =
      filterFactory->create(make_shared<StringReader>(text));
  shared_ptr<TokenStream> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  (std::static_pointer_cast<Tokenizer>(tokenStream))->setReader(filter);
  assertTokenStreamContents(
      tokenStream, std::deque<wstring>{L"時時馬鹿馬鹿しいところどころミスズ"});
}

void TestJapaneseIterationMarkCharFilterFactory::
    testIterationMarksWithJapaneseTokenizer() 
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));

  shared_ptr<JapaneseIterationMarkCharFilterFactory> filterFactory =
      make_shared<JapaneseIterationMarkCharFilterFactory>(
          unordered_map<wstring, wstring>());
  shared_ptr<CharFilter> filter = filterFactory->create(
      make_shared<StringReader>(L"時々馬鹿々々しいところゞゝゝミスヾ"));
  shared_ptr<TokenStream> tokenStream =
      tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(tokenStream))->setReader(filter);
  assertTokenStreamContents(
      tokenStream, std::deque<wstring>{L"時時", L"馬鹿馬鹿しい",
                                        L"ところどころ", L"ミ", L"スズ"});
}

void TestJapaneseIterationMarkCharFilterFactory::
    testKanjiOnlyIterationMarksWithJapaneseTokenizer() 
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));

  unordered_map<wstring, wstring> filterArgs =
      unordered_map<wstring, wstring>();
  filterArgs.emplace(L"normalizeKanji", L"true");
  filterArgs.emplace(L"normalizeKana", L"false");
  shared_ptr<JapaneseIterationMarkCharFilterFactory> filterFactory =
      make_shared<JapaneseIterationMarkCharFilterFactory>(filterArgs);

  shared_ptr<CharFilter> filter = filterFactory->create(
      make_shared<StringReader>(L"時々馬鹿々々しいところゞゝゝミスヾ"));
  shared_ptr<TokenStream> tokenStream =
      tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(tokenStream))->setReader(filter);
  assertTokenStreamContents(
      tokenStream, std::deque<wstring>{L"時時", L"馬鹿馬鹿しい", L"ところ",
                                        L"ゞ", L"ゝ", L"ゝ", L"ミス", L"ヾ"});
}

void TestJapaneseIterationMarkCharFilterFactory::
    testKanaOnlyIterationMarksWithJapaneseTokenizer() 
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));

  unordered_map<wstring, wstring> filterArgs =
      unordered_map<wstring, wstring>();
  filterArgs.emplace(L"normalizeKanji", L"false");
  filterArgs.emplace(L"normalizeKana", L"true");
  shared_ptr<JapaneseIterationMarkCharFilterFactory> filterFactory =
      make_shared<JapaneseIterationMarkCharFilterFactory>(filterArgs);

  shared_ptr<CharFilter> filter = filterFactory->create(
      make_shared<StringReader>(L"時々馬鹿々々しいところゞゝゝミスヾ"));
  shared_ptr<TokenStream> tokenStream =
      tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(tokenStream))->setReader(filter);
  assertTokenStreamContents(
      tokenStream, std::deque<wstring>{L"時々", L"馬鹿", L"々", L"々", L"しい",
                                        L"ところどころ", L"ミ", L"スズ"});
}

void TestJapaneseIterationMarkCharFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseIterationMarkCharFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseIterationMarkCharFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseIterationMarkCharFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja