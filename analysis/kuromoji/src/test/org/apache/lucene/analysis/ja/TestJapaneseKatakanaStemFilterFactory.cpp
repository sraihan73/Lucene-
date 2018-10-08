using namespace std;

#include "TestJapaneseKatakanaStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseKatakanaStemFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseKatakanaStemFilterFactory::testKatakanaStemming() throw(
    IOException)
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> tokenStream =
      tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(tokenStream))
      ->setReader(make_shared<StringReader>(
          L"明後日パーティーに行く予定がある。図書館で資料をコピーしました。"));
  shared_ptr<JapaneseKatakanaStemFilterFactory> filterFactory =
      make_shared<JapaneseKatakanaStemFilterFactory>(
          unordered_map<wstring, wstring>());
  assertTokenStreamContents(
      filterFactory->create(tokenStream),
      std::deque<wstring>{L"明後日", L"パーティ", L"に", L"行く", L"予定",
                           L"が", L"ある", L"図書館", L"で", L"資料", L"を",
                           L"コピー", L"し", L"まし", L"た"});
}

void TestJapaneseKatakanaStemFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseKatakanaStemFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseKatakanaStemFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseKatakanaStemFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja