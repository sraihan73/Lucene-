using namespace std;

#include "TestKeywordMarkerFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/CharTermAttribute.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/KeywordAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/PatternKeywordMarkerFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using CharTermAttribute =
    org::apache::lucene::analysis::tokenattributes::CharTermAttribute;
using KeywordAttribute =
    org::apache::lucene::analysis::tokenattributes::KeywordAttribute;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testSetFilterIncrementToken() throws
// java.io.IOException
void TestKeywordMarkerFilter::testSetFilterIncrementToken() 
{
  shared_ptr<CharArraySet> set = make_shared<CharArraySet>(5, true);
  set->add(L"lucenefox");
  std::deque<wstring> output = {L"the", L"quick", L"brown", L"LuceneFox",
                                 L"jumps"};
  assertTokenStreamContents(
      make_shared<LowerCaseFilterMock>(make_shared<SetKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"The quIck browN LuceneFox Jumps"), set)),
      output);
  shared_ptr<CharArraySet> mixedCaseSet =
      make_shared<CharArraySet>(asSet({L"LuceneFox"}), false);
  assertTokenStreamContents(
      make_shared<LowerCaseFilterMock>(make_shared<SetKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"The quIck browN LuceneFox Jumps"),
          mixedCaseSet)),
      output);
  shared_ptr<CharArraySet> set2 = set;
  assertTokenStreamContents(
      make_shared<LowerCaseFilterMock>(make_shared<SetKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"The quIck browN LuceneFox Jumps"), set2)),
      output);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testPatternFilterIncrementToken() throws
// java.io.IOException
void TestKeywordMarkerFilter::testPatternFilterIncrementToken() throw(
    IOException)
{
  std::deque<wstring> output = {L"the", L"quick", L"brown", L"LuceneFox",
                                 L"jumps"};
  assertTokenStreamContents(
      make_shared<LowerCaseFilterMock>(make_shared<PatternKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"The quIck browN LuceneFox Jumps"),
          Pattern::compile(L"[a-zA-Z]+[fF]ox"))),
      output);

  output =
      std::deque<wstring>{L"the", L"quick", L"brown", L"lucenefox", L"jumps"};

  assertTokenStreamContents(
      make_shared<LowerCaseFilterMock>(make_shared<PatternKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"The quIck browN LuceneFox Jumps"),
          Pattern::compile(L"[a-zA-Z]+[f]ox"))),
      output);
}

void TestKeywordMarkerFilter::testComposition() 
{
  shared_ptr<TokenStream> ts =
      make_shared<LowerCaseFilterMock>(make_shared<SetKeywordMarkerFilter>(
          make_shared<SetKeywordMarkerFilter>(
              whitespaceMockTokenizer(L"Dogs Trees Birds Houses"),
              make_shared<CharArraySet>(asSet({L"Birds", L"Houses"}), false)),
          make_shared<CharArraySet>(asSet({L"Dogs", L"Trees"}), false)));

  assertTokenStreamContents(
      ts, std::deque<wstring>{L"Dogs", L"Trees", L"Birds", L"Houses"});

  ts = make_shared<LowerCaseFilterMock>(make_shared<PatternKeywordMarkerFilter>(
      make_shared<PatternKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"Dogs Trees Birds Houses"),
          Pattern::compile(L"Birds|Houses")),
      Pattern::compile(L"Dogs|Trees")));

  assertTokenStreamContents(
      ts, std::deque<wstring>{L"Dogs", L"Trees", L"Birds", L"Houses"});

  ts = make_shared<LowerCaseFilterMock>(make_shared<SetKeywordMarkerFilter>(
      make_shared<PatternKeywordMarkerFilter>(
          whitespaceMockTokenizer(L"Dogs Trees Birds Houses"),
          Pattern::compile(L"Birds|Houses")),
      make_shared<CharArraySet>(asSet({L"Dogs", L"Trees"}), false)));

  assertTokenStreamContents(
      ts, std::deque<wstring>{L"Dogs", L"Trees", L"Birds", L"Houses"});
}

TestKeywordMarkerFilter::LowerCaseFilterMock::LowerCaseFilterMock(
    shared_ptr<TokenStream> in_)
    : org::apache::lucene::analysis::TokenFilter(in_)
{
}

bool TestKeywordMarkerFilter::LowerCaseFilterMock::incrementToken() throw(
    IOException)
{
  if (input->incrementToken()) {
    if (!keywordAttr->isKeyword()) {
      // C++ TODO: There is no native C++ equivalent to 'toString':
      const wstring term = termAtt->toString()->toLowerCase(Locale::ROOT);
      termAtt->setEmpty()->append(term);
    }
    return true;
  }
  return false;
}
} // namespace org::apache::lucene::analysis::miscellaneous