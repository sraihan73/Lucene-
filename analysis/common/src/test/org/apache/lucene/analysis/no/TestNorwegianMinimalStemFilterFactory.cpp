using namespace std;

#include "TestNorwegianMinimalStemFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"

namespace org::apache::lucene::analysis::no
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestNorwegianMinimalStemFilterFactory::testStemming() 
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"eple eplet epler eplene eplets eplenes");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"NorwegianMinimalStem").create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"epl", L"epl", L"epl", L"epl", L"epl", L"epl"});
}

void TestNorwegianMinimalStemFilterFactory::testBokmaalStemming() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"eple eplet epler eplene eplets eplenes");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"NorwegianMinimalStem", {L"variant", L"nb"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"epl", L"epl", L"epl", L"epl", L"epl", L"epl"});
}

void TestNorwegianMinimalStemFilterFactory::testNynorskStemming() throw(
    runtime_error)
{
  shared_ptr<Reader> reader =
      make_shared<StringReader>(L"gut guten gutar gutane gutens gutanes");
  shared_ptr<TokenStream> stream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  (std::static_pointer_cast<Tokenizer>(stream))->setReader(reader);
  stream = tokenFilterFactory(L"NorwegianMinimalStem", {L"variant", L"nn"})
               ->create(stream);
  assertTokenStreamContents(
      stream,
      std::deque<wstring>{L"gut", L"gut", L"gut", L"gut", L"gut", L"gut"});
}

void TestNorwegianMinimalStemFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"NorwegianMinimalStem", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::no