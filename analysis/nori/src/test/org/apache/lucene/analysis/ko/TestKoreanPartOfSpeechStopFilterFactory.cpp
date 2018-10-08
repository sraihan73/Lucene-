using namespace std;

#include "TestKoreanPartOfSpeechStopFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/Version.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanPartOfSpeechStopFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ko
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using Version = org::apache::lucene::util::Version;

void TestKoreanPartOfSpeechStopFilterFactory::testStopTags() 
{
  shared_ptr<KoreanTokenizerFactory> tokenizerFactory =
      make_shared<KoreanTokenizerFactory>(unordered_map<wstring, wstring>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> ts = tokenizerFactory->create();
  (std::static_pointer_cast<Tokenizer>(ts))
      ->setReader(make_shared<StringReader>(L" 한국은 대단한 나라입니다."));
  unordered_map<wstring, wstring> args = unordered_map<wstring, wstring>();
  // C++ TODO: There is no native C++ equivalent to 'toString':
  args.emplace(L"luceneMatchVersion", Version::LATEST->toString());
  args.emplace(L"tags", L"E, J");
  shared_ptr<KoreanPartOfSpeechStopFilterFactory> factory =
      make_shared<KoreanPartOfSpeechStopFilterFactory>(args);
  ts = factory->create(ts);
  assertTokenStreamContents(
      ts, std::deque<wstring>{L"한국", L"대단", L"하", L"나라", L"이"});
}

void TestKoreanPartOfSpeechStopFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KoreanPartOfSpeechStopFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestKoreanPartOfSpeechStopFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestKoreanPartOfSpeechStopFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  // C++ TODO: There is no native C++ equivalent to 'toString':
  this->put(L"luceneMatchVersion", Version::LATEST->toString());
  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ko