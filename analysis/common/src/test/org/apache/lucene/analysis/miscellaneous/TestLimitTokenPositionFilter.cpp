using namespace std;

#include "TestLimitTokenPositionFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRef.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/CharsRefBuilder.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LimitTokenPositionFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/synonym/SynonymMap.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using SynonymFilter = org::apache::lucene::analysis::synonym::SynonymFilter;
using SynonymMap = org::apache::lucene::analysis::synonym::SynonymMap;
using CharsRef = org::apache::lucene::util::CharsRef;
using CharsRefBuilder = org::apache::lucene::util::CharsRefBuilder;
using org::junit::Test;

void TestLimitTokenPositionFilter::testMaxPosition2() 
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass>(
        shared_from_this(), consumeAll);

    // don't use assertAnalyzesTo here, as the end offset is not the end of the
    // string (unless consumeAll is true, in which case it's correct)!
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  2     3  4  5"),
                              std::deque<wstring>{L"1", L"2"},
                              std::deque<int>{0, 3}, std::deque<int>{1, 4},
                              consumeAll ? 16 : nullptr);
    assertTokenStreamContents(
        a->tokenStream(L"dummy", make_shared<StringReader>(L"1 2 3 4 5")),
        std::deque<wstring>{L"1", L"2"}, std::deque<int>{0, 2},
        std::deque<int>{1, 3}, consumeAll ? 9 : nullptr);

    // less than the limit, ensure we behave correctly
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  "),
                              std::deque<wstring>{L"1"}, std::deque<int>{0},
                              std::deque<int>{1}, consumeAll ? 3 : nullptr);

    // equal to limit
    assertTokenStreamContents(a->tokenStream(L"dummy", L"1  2  "),
                              std::deque<wstring>{L"1", L"2"},
                              std::deque<int>{0, 3}, std::deque<int>{1, 4},
                              consumeAll ? 6 : nullptr);
    delete a;
  }
}

TestLimitTokenPositionFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestLimitTokenPositionFilter> outerInstance, bool consumeAll)
{
  this->outerInstance = outerInstance;
  this->consumeAll = consumeAll;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestLimitTokenPositionFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<MockTokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  // if we are consuming all tokens, we can use the checks, otherwise we can't
  tokenizer->setEnableChecks(consumeAll);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer,
      make_shared<LimitTokenPositionFilter>(tokenizer, 2, consumeAll));
}

void TestLimitTokenPositionFilter::testMaxPosition3WithSynomyms() throw(
    IOException)
{
  for (auto consumeAll : std::deque<bool>{true, false}) {
    shared_ptr<MockTokenizer> tokenizer =
        whitespaceMockTokenizer(L"one two three four five");
    // if we are consuming all tokens, we can use the checks, otherwise we can't
    tokenizer->setEnableChecks(consumeAll);

    shared_ptr<SynonymMap::Builder> builder =
        make_shared<SynonymMap::Builder>(true);
    builder->add(make_shared<CharsRef>(L"one"), make_shared<CharsRef>(L"first"),
                 true);
    builder->add(make_shared<CharsRef>(L"one"), make_shared<CharsRef>(L"alpha"),
                 true);
    builder->add(make_shared<CharsRef>(L"one"),
                 make_shared<CharsRef>(L"beguine"), true);
    shared_ptr<CharsRefBuilder> multiWordCharsRef =
        make_shared<CharsRefBuilder>();
    SynonymMap::Builder::join(
        std::deque<wstring>{L"and", L"indubitably", L"single", L"only"},
        multiWordCharsRef);
    builder->add(make_shared<CharsRef>(L"one"), multiWordCharsRef->get(), true);
    SynonymMap::Builder::join(std::deque<wstring>{L"dopple", L"ganger"},
                              multiWordCharsRef);
    builder->add(make_shared<CharsRef>(L"two"), multiWordCharsRef->get(), true);
    shared_ptr<SynonymMap> synonymMap = builder->build();
    shared_ptr<TokenStream> stream =
        make_shared<SynonymFilter>(tokenizer, synonymMap, true);
    stream = make_shared<LimitTokenPositionFilter>(stream, 3, consumeAll);

    // "only", the 4th word of multi-word synonym "and indubitably single only"
    // is not emitted, since its position is greater than 3.
    assertTokenStreamContents(
        stream,
        std::deque<wstring>{L"one", L"first", L"alpha", L"beguine", L"and",
                             L"two", L"indubitably", L"dopple", L"three",
                             L"single", L"ganger"},
        std::deque<int>{1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0});
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments() throws Exception
void TestLimitTokenPositionFilter::testIllegalArguments() 
{
  make_shared<LimitTokenPositionFilter>(
      whitespaceMockTokenizer(L"one two three four five"), 0);
}
} // namespace org::apache::lucene::analysis::miscellaneous