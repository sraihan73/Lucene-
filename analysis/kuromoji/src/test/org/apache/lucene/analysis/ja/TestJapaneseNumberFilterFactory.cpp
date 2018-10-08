using namespace std;

#include "TestJapaneseNumberFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseNumberFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestJapaneseNumberFilterFactory::testBasics() 
{

  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  args.emplace(L"discardPunctuation", L"false");

  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(args);

  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> tokenStream =
      tokenizerFactory->create(newAttributeFactory());
  (std::static_pointer_cast<Tokenizer>(tokenStream))
      ->setReader(make_shared<StringReader>(L"昨日のお寿司は1０万円でした。"));

  shared_ptr<JapaneseNumberFilterFactory> factory =
      make_shared<JapaneseNumberFilterFactory>(unordered_map<>());
  tokenStream = factory->create(tokenStream);
  assertTokenStreamContents(tokenStream,
                            std::deque<wstring>{L"昨日", L"の", L"お", L"寿司",
                                                 L"は", L"100000", L"円",
                                                 L"でし", L"た", L"。"});
}

void TestJapaneseNumberFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapaneseNumberFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapaneseNumberFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapaneseNumberFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja