using namespace std;

#include "TestJapanesePartOfSpeechStopFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapanesePartOfSpeechStopFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ja/JapaneseTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ja
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Version = org::apache::lucene::util::Version;

void TestJapanesePartOfSpeechStopFilterFactory::testBasics() 
{
  wstring tags = wstring(L"#  verb-main:\n") + L"動詞-自立\n";

  shared_ptr<JapaneseTokenizerFactory> tokenizerFactory =
      make_shared<JapaneseTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = tokenizerFactory->create();
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L"私は制限スピードを超える。"));
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  args.emplace(L"luceneMatchVersion", Version::LATEST->toString());
  args.emplace(L"tags", L"stoptags.txt");
  shared_ptr<JapanesePartOfSpeechStopFilterFactory> factory =
      make_shared<JapanesePartOfSpeechStopFilterFactory>(args);
  factory->inform(make_shared<StringMockResourceLoader>(tags));
  ts = factory->create(ts);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"私", L"は", L"制限", L"スピード", L"を"});
}

void TestJapanesePartOfSpeechStopFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<JapanesePartOfSpeechStopFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestJapanesePartOfSpeechStopFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestJapanesePartOfSpeechStopFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(L"luceneMatchVersion", Version::LATEST->toString());
  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ja