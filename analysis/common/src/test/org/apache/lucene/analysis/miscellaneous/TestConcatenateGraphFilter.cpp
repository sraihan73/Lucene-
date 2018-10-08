using namespace std;

#include "TestConcatenateGraphFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/StopFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using StopFilter = org::apache::lucene::analysis::StopFilter;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using SynonymFilter = org::apache::lucene::analysis::synonym::SynonymFilter;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasic() throws Exception
void TestConcatenateGraphFilter::testBasic() 
{
  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  wstring input = L"mykeyword";
  tokenStream->setReader(make_shared<StringReader>(input));
  shared_ptr<ConcatenateGraphFilter> stream =
      make_shared<ConcatenateGraphFilter>(tokenStream);
  assertTokenStreamContents(stream, std::deque<wstring>{input}, nullptr,
                            nullptr, std::deque<int>{1});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithNoPreserveSep() throws Exception
void TestConcatenateGraphFilter::testWithNoPreserveSep() 
{
  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  wstring input = L"mykeyword another keyword";
  tokenStream->setReader(make_shared<StringReader>(input));
  shared_ptr<ConcatenateGraphFilter> stream =
      make_shared<ConcatenateGraphFilter>(tokenStream, false, false, 100);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"mykeywordanotherkeyword"},
                            nullptr, nullptr, std::deque<int>{1});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithMultipleTokens() throws Exception
void TestConcatenateGraphFilter::testWithMultipleTokens() 
{
  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  wstring input = L"mykeyword another keyword";
  tokenStream->setReader(make_shared<StringReader>(input));
  shared_ptr<ConcatenateGraphFilter> stream =
      make_shared<ConcatenateGraphFilter>(tokenStream);
  shared_ptr<CharsRefBuilder> builder = make_shared<CharsRefBuilder>();
  builder->append(L"mykeyword");
  builder->append(SEP_LABEL);
  builder->append(L"another");
  builder->append(SEP_LABEL);
  builder->append(L"keyword");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertTokenStreamContents(
      stream, std::deque<wstring>{builder->toCharsRef()->toString()}, nullptr,
      nullptr, std::deque<int>{1});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithSynonym() throws Exception
void TestConcatenateGraphFilter::testWithSynonym() 
{
  shared_ptr<SynonymMap::Builder> builder =
      make_shared<SynonymMap::Builder>(true);
  builder->add(make_shared<CharsRef>(L"mykeyword"),
               make_shared<CharsRef>(L"mysynonym"), true);
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokenizer->setReader(make_shared<StringReader>(L"mykeyword"));
  shared_ptr<SynonymFilter> filter =
      make_shared<SynonymFilter>(tokenizer, builder->build(), true);
  shared_ptr<ConcatenateGraphFilter> stream =
      make_shared<ConcatenateGraphFilter>(filter);
  assertTokenStreamContents(stream,
                            std::deque<wstring>{L"mykeyword", L"mysynonym"},
                            nullptr, nullptr, std::deque<int>{1, 0});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithSynonyms() throws Exception
void TestConcatenateGraphFilter::testWithSynonyms() 
{
  shared_ptr<SynonymMap::Builder> builder =
      make_shared<SynonymMap::Builder>(true);
  builder->add(make_shared<CharsRef>(L"mykeyword"),
               make_shared<CharsRef>(L"mysynonym"), true);
  shared_ptr<Tokenizer> tokenStream =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  wstring input = L"mykeyword another keyword";
  tokenStream->setReader(make_shared<StringReader>(input));
  shared_ptr<SynonymFilter> filter =
      make_shared<SynonymFilter>(tokenStream, builder->build(), true);
  shared_ptr<ConcatenateGraphFilter> stream =
      make_shared<ConcatenateGraphFilter>(filter, true, false, 100);
  std::deque<wstring> expectedOutputs(2);
  shared_ptr<CharsRefBuilder> expectedOutput = make_shared<CharsRefBuilder>();
  expectedOutput->append(L"mykeyword");
  expectedOutput->append(SEP_LABEL);
  expectedOutput->append(L"another");
  expectedOutput->append(SEP_LABEL);
  expectedOutput->append(L"keyword");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  expectedOutputs[0] = expectedOutput->toCharsRef()->toString();
  expectedOutput->clear();
  expectedOutput->append(L"mysynonym");
  expectedOutput->append(SEP_LABEL);
  expectedOutput->append(L"another");
  expectedOutput->append(SEP_LABEL);
  expectedOutput->append(L"keyword");
  // C++ TODO: There is no native C++ equivalent to 'toString':
  expectedOutputs[1] = expectedOutput->toCharsRef()->toString();
  assertTokenStreamContents(stream, expectedOutputs, nullptr, nullptr,
                            std::deque<int>{1, 0});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testWithStopword() throws Exception
void TestConcatenateGraphFilter::testWithStopword() 
{
  for (auto preservePosInc : std::deque<bool>{true, false}) {
    shared_ptr<Tokenizer> tokenStream =
        make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
    wstring input = L"a mykeyword a keyword"; // LUCENE-8344 add "a"
    tokenStream->setReader(make_shared<StringReader>(input));
    shared_ptr<TokenFilter> tokenFilter =
        make_shared<StopFilter>(tokenStream, StopFilter::makeStopSet({L"a"}));
    shared_ptr<ConcatenateGraphFilter> concatStream =
        make_shared<ConcatenateGraphFilter>(tokenFilter, true, preservePosInc,
                                            10);
    shared_ptr<CharsRefBuilder> builder = make_shared<CharsRefBuilder>();
    if (preservePosInc) {
      builder->append(SEP_LABEL);
    }
    builder->append(L"mykeyword");
    builder->append(SEP_LABEL);
    if (preservePosInc) {
      builder->append(SEP_LABEL);
    }
    builder->append(L"keyword");
    //      if (preservePosInc) { LUCENE-8344 uncomment
    //        builder.append(SEP_LABEL);
    //      }
    // C++ TODO: There is no native C++ equivalent to 'toString':
    assertTokenStreamContents(
        concatStream, std::deque<wstring>{builder->toCharsRef()->toString()});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testValidNumberOfExpansions() throws
// java.io.IOException
void TestConcatenateGraphFilter::testValidNumberOfExpansions() throw(
    IOException)
{
  shared_ptr<SynonymMap::Builder> builder =
      make_shared<SynonymMap::Builder>(true);
  for (int i = 0; i < 256; i++) {
    builder->add(make_shared<CharsRef>(L"" + to_wstring(i + 1)),
                 make_shared<CharsRef>(L"" + to_wstring(1000 + (i + 1))), true);
  }
  shared_ptr<StringBuilder> valueBuilder = make_shared<StringBuilder>();
  for (int i = 0; i < 8; i++) {
    valueBuilder->append(i + 1);
    valueBuilder->append(L" ");
  }
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, true);
  tokenizer->setReader(make_shared<StringReader>(valueBuilder->toString()));
  shared_ptr<SynonymFilter> filter =
      make_shared<SynonymFilter>(tokenizer, builder->build(), true);

  int count;
  // C++ NOTE: The following 'try with resources' block is replaced by its C++
  // equivalent: ORIGINAL LINE: try (ConcatenateGraphFilter stream = new
  // ConcatenateGraphFilter(filter))
  {
    ConcatenateGraphFilter stream = ConcatenateGraphFilter(filter);
    stream->reset();
    shared_ptr<ConcatenateGraphFilter::BytesRefBuilderTermAttribute> attr =
        stream->addAttribute(
            ConcatenateGraphFilter::BytesRefBuilderTermAttribute::typeid);
    count = 0;
    while (stream->incrementToken()) {
      count++;
      assertNotNull(attr->getBytesRef());
      assertTrue(attr->getBytesRef()->length > 0);
    }
  }
  assertEquals(count, 256);
}

void TestConcatenateGraphFilter::testEmpty() 
{
  shared_ptr<Tokenizer> tokenizer = whitespaceMockTokenizer(L"");
  shared_ptr<ConcatenateGraphFilter> filter =
      make_shared<ConcatenateGraphFilter>(tokenizer);
  assertTokenStreamContents(filter, std::deque<wstring>(0));
}
} // namespace org::apache::lucene::analysis::miscellaneous