using namespace std;

#include "TestKoreanReadingFormFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanReadingFormFilterFactory.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizerFactory.h"
#include "StringMockResourceLoader.h"

namespace org::apache::lucene::analysis::ko
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestKoreanReadingFormFilterFactory::testReadings() 
{
  shared_ptr<KoreanTokenizerFactory> tokenizerFactory =
      make_shared<KoreanTokenizerFactory>(unordered_map<>());
  tokenizerFactory->inform(make_shared<StringMockResourceLoader>(L""));
  shared_ptr<TokenStream> tokenStream = tokenizerFactory->create();
  (std::static_pointer_cast<Tokenizer>(tokenStream))
      ->setReader(make_shared<StringReader>(L"丞相"));
  shared_ptr<KoreanReadingFormFilterFactory> filterFactory =
      make_shared<KoreanReadingFormFilterFactory>(unordered_map<>());
  assertTokenStreamContents(filterFactory->create(tokenStream),
                            std::deque<wstring>{L"승상"});
}

void TestKoreanReadingFormFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<KoreanReadingFormFilterFactory>(
        make_shared<HashMapAnonymousInnerClass>(shared_from_this()));
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

TestKoreanReadingFormFilterFactory::HashMapAnonymousInnerClass::
    HashMapAnonymousInnerClass(
        shared_ptr<TestKoreanReadingFormFilterFactory> outerInstance)
{
  this->outerInstance = outerInstance;

  this->put(L"bogusArg", L"bogusValue");
}
} // namespace org::apache::lucene::analysis::ko