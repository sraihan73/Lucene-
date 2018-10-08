using namespace std;

#include "LengthGoalBreakIteratorTest.h"

namespace org::apache::lucene::search::uhighlight
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using MockAnalyzer = org::apache::lucene::analysis::MockAnalyzer;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Query = org::apache::lucene::search::Query;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using QueryBuilder = org::apache::lucene::util::QueryBuilder;
const wstring LengthGoalBreakIteratorTest::FIELD = L"body";
const wstring LengthGoalBreakIteratorTest::CONTENT = L"Aa bb. Cc dd. Ee ff";

void LengthGoalBreakIteratorTest::testTargetLen() 
{
  // "goal" means target length goal to find closest break

  // at first word:
  shared_ptr<Query> query = this->query(L"aa");
  assertEquals(L"almost two sent", L"<b>Aa</b> bb.",
               highlightClosestToLen(CONTENT, query, 9));
  assertEquals(L"barely two sent", L"<b>Aa</b> bb. Cc dd.",
               highlightClosestToLen(CONTENT, query, 10));
  assertEquals(
      L"long goal", L"<b>Aa</b> bb. Cc dd. Ee ff",
      highlightClosestToLen(CONTENT, query, 17 + random()->nextInt(20)));

  // at some word not at start of passage
  query = this->query(L"dd");
  assertEquals(L"short goal", L" Cc <b>dd</b>.",
               highlightClosestToLen(CONTENT, query, random()->nextInt(5)));
  assertEquals(L"almost two sent", L" Cc <b>dd</b>.",
               highlightClosestToLen(CONTENT, query, 10));
  assertEquals(L"barely two sent", L" Cc <b>dd</b>. Ee ff",
               highlightClosestToLen(CONTENT, query, 11));
  assertEquals(
      L"long goal", L" Cc <b>dd</b>. Ee ff",
      highlightClosestToLen(CONTENT, query, 12 + random()->nextInt(20)));
}

void LengthGoalBreakIteratorTest::testMinLen() 
{
  // minLen mode is simpler than targetLen... just test a few cases

  shared_ptr<Query> query = this->query(L"dd");
  assertEquals(L"almost two sent", L" Cc <b>dd</b>.",
               highlightMinLen(CONTENT, query, 6));
  assertEquals(L"barely two sent", L" Cc <b>dd</b>. Ee ff",
               highlightMinLen(CONTENT, query, 7));
}

void LengthGoalBreakIteratorTest::testDefaultSummaryTargetLen() throw(
    IOException)
{
  shared_ptr<Query> query = this->query(L"zz");
  assertEquals(L"Aa bb.", highlightClosestToLen(CONTENT, query,
                                                random()->nextInt(10))); // < 10
  assertEquals(L"Aa bb. Cc dd.",
               highlightClosestToLen(CONTENT, query,
                                     10 + 6)); // cusp of adding 3rd sentence
  assertEquals(L"Aa bb. Cc dd. Ee ff",
               highlightClosestToLen(CONTENT, query,
                                     17 + random()->nextInt(20))); // >= 14
}

shared_ptr<Query> LengthGoalBreakIteratorTest::query(const wstring &qStr)
{
  return (make_shared<QueryBuilder>(analyzer))->createBooleanQuery(FIELD, qStr);
}

wstring LengthGoalBreakIteratorTest::highlightClosestToLen(
    const wstring &content, shared_ptr<Query> query,
    int lengthGoal) 
{
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(nullptr, analyzer);
  highlighter->setBreakIterator([&]() {
    LengthGoalBreakIterator::createClosestToLength(
        make_shared<CustomSeparatorBreakIterator>(L'.'), lengthGoal);
  });
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return highlighter->highlightWithoutSearcher(FIELD, query, content, 1)
      .toString();
}

wstring
LengthGoalBreakIteratorTest::highlightMinLen(const wstring &content,
                                             shared_ptr<Query> query,
                                             int lengthGoal) 
{
  // differs from above only by "createMinLength"
  shared_ptr<UnifiedHighlighter> highlighter =
      make_shared<UnifiedHighlighter>(nullptr, analyzer);
  highlighter->setBreakIterator([&]() {
    LengthGoalBreakIterator::createMinLength(
        make_shared<CustomSeparatorBreakIterator>(L'.'), lengthGoal);
  });
  // C++ TODO: There is no native C++ equivalent to 'toString':
  return highlighter->highlightWithoutSearcher(FIELD, query, content, 1)
      .toString();
}
} // namespace org::apache::lucene::search::uhighlight