using namespace std;

#include "TestGermanStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::de
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestGermanStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader = make_shared<StringReader>(L"Tischen");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"GermanStem").create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"tisch"});
}

void TestGermanStemFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"GermanStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::de