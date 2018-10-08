using namespace std;

#include "TestSpanishLightStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::es
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestSpanishLightStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"sociedades");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"SpanishLightStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"sociedad"});
}

void TestSpanishLightStemFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"SpanishLightStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::es