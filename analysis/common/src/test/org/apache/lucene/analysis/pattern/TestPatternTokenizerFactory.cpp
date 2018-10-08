using namespace std;

#include "TestPatternTokenizerFactory.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"

namespace org::apache::lucene::analysis::pattern
{
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using BaseTokenStreamFactoryTestCase =
    org::apache::lucene::analysis::util::BaseTokenStreamFactoryTestCase;

void TestPatternTokenizerFactory::testFactory() 
{
  shared_ptr<Reader> *const reader =
      make_shared<StringReader>(L"Günther Günther is here");
  // create PatternTokenizer
  shared_ptr<Tokenizer> stream =
      tokenizerFactory(L"Pattern", {L"pattern", L"[,;/\\s]+"})
          ->create(newAttributeFactory());
  stream->setReader(reader);
  assertTokenStreamContents(
      stream, std::deque<wstring>{L"Günther", L"Günther", L"is", L"here"});
}

void TestPatternTokenizerFactory::testBogusArguments() 
{
  invalid_argument expected = expectThrows(invalid_argument::typeid, [&]() {
    tokenizerFactory(L"Pattern",
                     {L"pattern", L"something", L"bogusArg", L"bogusValue"});
  });
  assertTrue(expected.what()->contains(L"Unknown parameters"));
}
} // namespace org::apache::lucene::analysis::pattern