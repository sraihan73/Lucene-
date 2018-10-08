using namespace std;

#include "TestJapaneseBaseFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseBaseFormFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseBaseFormFilterFactory::testBasics() 
{
  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"それはまだ実験段階にあります"));
  shared_ptr<JapaneseBaseFormFilterFactory> factory =
      make_shared<JapaneseBaseFormFilterFactory>(
          unordered_map<wstring, wstring>());
  ts = factory->create(ts);
  assertTokenStreamContents(ts, std::deque<wstring>{L"それ", L"は", L"まだ",
                                                     L"実験", L"段階", L"に",
                                                     L"ある", L"ます"});
}

void TestJapaneseBaseFormFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseBaseFormFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseBaseFormFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseBaseFormFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja