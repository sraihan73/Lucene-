using namespace std;

#include "TestCodepointCountFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestCodepointCountFilterFactory::testPositionIncrements() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"foo foobar super-duper-trooper");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"CodepointCount", {L"min", L"4", L"max", L"10"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{L"foobar"},
                            std::deque<int>{2});
}

void TestCodepointCountFilterFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"CodepointCount", {L"min", L"4", L"max", L"5",
                                           L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}

void TestCodepointCountFilterFactory::testInvalidArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    shared_ptr<Reader> reader =
        make_shared<StringReader>(L"foo foobar super-duper-trooper");
    shared_ptr<TokenStream> stream =
        make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
    (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
    tokenFilterFactory(L"CodepointCount",
                       {CodepointCountFilterFactory::MIN_KEY, L"5",
                        CodepointCountFilterFactory::MAX_KEY, L"4"})
        ->create(stream);
  });
  assertTrue(expected.what()->contains(
      L"maximum length must not be greater than minimum length"));
}
} // namespace org::apache::lucene::analysis::miscellaneous