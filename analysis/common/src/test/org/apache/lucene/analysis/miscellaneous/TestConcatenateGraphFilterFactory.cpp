using namespace std;

#include "TestConcatenateGraphFilterFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/ConcatenateGraphFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestConcatenateGraphFilterFactory::test() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    const wstring input = L"A1 B2 A1 D4 C3";
    shared_ptr<Reader> reader = make_shared<StringReader>(input);
    shared_ptr<MockTokenizer> tokenizer =
        make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
    tokenizer->setReader(reader);
    tokenizer->setEnableChecks(consumeAll);
    shared_ptr<TokenStream> stream = tokenizer;
    stream = tokenFilterFactory(L"ConcatenateGraph").create(stream);
    assertTokenStreamContents(
        stream, std::deque<wstring>{StringHelper::replace(
                    input, L' ',
                    static_cast<wchar_t>(ConcatenateGraphFilter::SEP_LABEL))});
  }
}

void TestConcatenateGraphFilterFactory::testPreserveSep() 
{
  const wstring input = L"A1 B2 A1 D4 C3";
  const wstring output = L"A1A1D4C3";
  shared_ptr<Reader> reader = make_shared<StringReader>(input);
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(reader);
  shared_ptr<TokenStream> stream = tokenizer;
  stream = make_shared<StopFilter>(stream, StopFilter::makeStopSet({L"B2"}));
  stream = tokenFilterFactory(L"ConcatenateGraph", {L"preserveSep", L"false"})
               ->create(stream);
  assertTokenStreamContents(stream, std::deque<wstring>{output});
}

void TestConcatenateGraphFilterFactory::testPreservePositionIncrements() throw(
    runtime_error)
{
  const wstring input = L"A1 B2 A1 D4 C3";
  const wstring output = L"A1 A1 D4 C3";
  shared_ptr<Reader> reader = make_shared<StringReader>(input);
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(reader);
  shared_ptr<TokenStream> stream = tokenizer;
  stream = make_shared<StopFilter>(stream, StopFilter::makeStopSet({L"B2"}));
  stream = tokenFilterFactory(L"ConcatenateGraph",
                              {L"preservePositionIncrements", L"false"})
               ->create(stream);
  assertTokenStreamContents(
      stream, std::deque<wstring>{StringHelper::replace(
                  output, L' ',
                  static_cast<wchar_t>(ConcatenateGraphFilter::SEP_LABEL))});
}

void TestConcatenateGraphFilterFactory::testRequired() 
{
  // no params are required
  tokenFilterFactory(L"ConcatenateGraph");
}

void TestConcatenateGraphFilterFactory::testBogusArguments() throw(
    runtime_error)
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenFilterFactory(L"ConcatenateGraph", {L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::miscellaneous