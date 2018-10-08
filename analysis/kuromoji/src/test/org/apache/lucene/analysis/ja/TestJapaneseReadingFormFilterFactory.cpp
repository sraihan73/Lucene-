using namespace std;

#include "TestJapaneseReadingFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseReadingFormFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseReadingFormFilterFactory::testReadings() 
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> tokenStream = tokenizerFactory->create();
  (std::static_pointer_cast<Tokenizer>(tokenStream))
      ->setReader(make_shared<StringReader>(L"先ほどベルリンから来ました。"));
  shared_ptr<JapaneseReadingFormFilterFactory> filterFactory =
      make_shared<JapaneseReadingFormFilterFactory>(
          unordered_map<wstring, wstring>());
  assertTokenStreamContents(filterFactory->create(tokenStream),
                            std::deque<wstring>{L"サキ", L"ホド", L"ベルリン",
                                                 L"カラ", L"キ", L"マシ",
                                                 L"タ"});
}

void TestJapaneseReadingFormFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseReadingFormFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseReadingFormFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseReadingFormFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja