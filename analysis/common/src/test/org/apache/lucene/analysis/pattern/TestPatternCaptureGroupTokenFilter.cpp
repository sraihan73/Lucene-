using namespace std;

#include "TestPatternCaptureGroupTokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/pattern/PatternCaptureGroupTokenFilter.h"

namespace org::apache::lucene::analysis::pattern
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;

void TestPatternCaptureGroupTokenFilter::testNoPattern() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>(),
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>(),
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>(),
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>(),
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testNoMatch() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"xx"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"xx"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"xx"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"xx"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testNoCapture() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L".."},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L".."},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L".."},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L".."},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testEmptyCapture() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L".(y*)"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L".(y*)"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L".(y*)"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L".(y*)"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureAll() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.+)"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.+)"},
               std::deque<wstring>{L"foobarbaz"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.+)"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.+)"},
               std::deque<wstring>{L"foo", L"bar", L"baz"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureStart() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.)"},
               std::deque<wstring>{L"f"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.)"},
               std::deque<wstring>{L"foobarbaz", L"f"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.)"},
               std::deque<wstring>{L"f", L"b", L"b"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.)"},
               std::deque<wstring>{L"foo", L"f", L"bar", L"b", L"baz", L"b"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureMiddle() throw(
    runtime_error)
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^.(.)."},
               std::deque<wstring>{L"o"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^.(.)."},
               std::deque<wstring>{L"foobarbaz", L"o"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^.(.)."},
               std::deque<wstring>{L"o", L"a", L"a"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^.(.)."},
               std::deque<wstring>{L"foo", L"o", L"bar", L"a", L"baz", L"a"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureEnd() 
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.)$"},
               std::deque<wstring>{L"z"}, std::deque<int>{0},
               std::deque<int>{9}, std::deque<int>{1}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.)$"},
               std::deque<wstring>{L"foobarbaz", L"z"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.)$"},
               std::deque<wstring>{L"o", L"r", L"z"},
               std::deque<int>{0, 4, 8}, std::deque<int>{3, 7, 11},
               std::deque<int>{1, 1, 1}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.)$"},
               std::deque<wstring>{L"foo", L"o", L"bar", L"r", L"baz", L"z"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureStartMiddle() throw(
    runtime_error)
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.)(.)"},
               std::deque<wstring>{L"f", L"o"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.)(.)"},
               std::deque<wstring>{L"foobarbaz", L"f", L"o"},
               std::deque<int>{0, 0, 0}, std::deque<int>{9, 9, 9},
               std::deque<int>{1, 0, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.)(.)"},
               std::deque<wstring>{L"f", L"o", L"b", L"a", L"b", L"a"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.)(.)"},
               std::deque<wstring>{L"foo", L"f", L"o", L"bar", L"b", L"a",
                                    L"baz", L"b", L"a"},
               std::deque<int>{0, 0, 0, 4, 4, 4, 8, 8, 8},
               std::deque<int>{3, 3, 3, 7, 7, 7, 11, 11, 11},
               std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureStartEnd() throw(
    runtime_error)
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.).+(.)$"},
               std::deque<wstring>{L"f", L"z"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"^(.).+(.)$"},
               std::deque<wstring>{L"foobarbaz", L"f", L"z"},
               std::deque<int>{0, 0, 0}, std::deque<int>{9, 9, 9},
               std::deque<int>{1, 0, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.).+(.)$"},
               std::deque<wstring>{L"f", L"o", L"b", L"r", L"b", L"z"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"^(.).+(.)$"},
               std::deque<wstring>{L"foo", L"f", L"o", L"bar", L"b", L"r",
                                    L"baz", L"b", L"z"},
               std::deque<int>{0, 0, 0, 4, 4, 4, 8, 8, 8},
               std::deque<int>{3, 3, 3, 7, 7, 7, 11, 11, 11},
               std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testCaptureMiddleEnd() throw(
    runtime_error)
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.)(.)$"},
               std::deque<wstring>{L"a", L"z"}, std::deque<int>{0, 0},
               std::deque<int>{9, 9}, std::deque<int>{1, 0}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.)(.)$"},
               std::deque<wstring>{L"foobarbaz", L"a", L"z"},
               std::deque<int>{0, 0, 0}, std::deque<int>{9, 9, 9},
               std::deque<int>{1, 0, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.)(.)$"},
               std::deque<wstring>{L"o", L"o", L"a", L"r", L"a", L"z"},
               std::deque<int>{0, 0, 4, 4, 8, 8},
               std::deque<int>{3, 3, 7, 7, 11, 11},
               std::deque<int>{1, 0, 1, 0, 1, 0}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.)(.)$"},
               std::deque<wstring>{L"foo", L"o", L"o", L"bar", L"a", L"r",
                                    L"baz", L"a", L"z"},
               std::deque<int>{0, 0, 0, 4, 4, 4, 8, 8, 8},
               std::deque<int>{3, 3, 3, 7, 7, 7, 11, 11, 11},
               std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testMultiCaptureOverlap() throw(
    runtime_error)
{
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.(.(.)))"},
               std::deque<wstring>{L"foo", L"oo", L"o", L"bar", L"ar", L"r",
                                    L"baz", L"az", L"z"},
               std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0},
               std::deque<int>{9, 9, 9, 9, 9, 9, 9, 9, 9},
               std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0}, false);
  testPatterns(L"foobarbaz", std::deque<wstring>{L"(.(.(.)))"},
               std::deque<wstring>{L"foobarbaz", L"foo", L"oo", L"o", L"bar",
                                    L"ar", L"r", L"baz", L"az", L"z"},
               std::deque<int>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
               std::deque<int>{9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
               std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, true);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.(.(.)))"},
               std::deque<wstring>{L"foo", L"oo", L"o", L"bar", L"ar", L"r",
                                    L"baz", L"az", L"z"},
               std::deque<int>{0, 0, 0, 4, 4, 4, 8, 8, 8},
               std::deque<int>{3, 3, 3, 7, 7, 7, 11, 11, 11},
               std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0}, false);

  testPatterns(L"foo bar baz", std::deque<wstring>{L"(.(.(.)))"},
               std::deque<wstring>{L"foo", L"oo", L"o", L"bar", L"ar", L"r",
                                    L"baz", L"az", L"z"},
               std::deque<int>{0, 0, 0, 4, 4, 4, 8, 8, 8},
               std::deque<int>{3, 3, 3, 7, 7, 7, 11, 11, 11},
               std::deque<int>{1, 0, 0, 1, 0, 0, 1, 0, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testMultiPattern() 
{
  testPatterns(L"aaabbbaaa", std::deque<wstring>{L"(aaa)", L"(bbb)", L"(ccc)"},
               std::deque<wstring>{L"aaa", L"bbb", L"aaa"},
               std::deque<int>{0, 0, 0}, std::deque<int>{9, 9, 9},
               std::deque<int>{1, 0, 0}, false);
  testPatterns(L"aaabbbaaa", std::deque<wstring>{L"(aaa)", L"(bbb)", L"(ccc)"},
               std::deque<wstring>{L"aaabbbaaa", L"aaa", L"bbb", L"aaa"},
               std::deque<int>{0, 0, 0, 0}, std::deque<int>{9, 9, 9, 9},
               std::deque<int>{1, 0, 0, 0}, true);

  testPatterns(
      L"aaa bbb aaa", std::deque<wstring>{L"(aaa)", L"(bbb)", L"(ccc)"},
      std::deque<wstring>{L"aaa", L"bbb", L"aaa"}, std::deque<int>{0, 4, 8},
      std::deque<int>{3, 7, 11}, std::deque<int>{1, 1, 1}, false);

  testPatterns(
      L"aaa bbb aaa", std::deque<wstring>{L"(aaa)", L"(bbb)", L"(ccc)"},
      std::deque<wstring>{L"aaa", L"bbb", L"aaa"}, std::deque<int>{0, 4, 8},
      std::deque<int>{3, 7, 11}, std::deque<int>{1, 1, 1}, true);
}

void TestPatternCaptureGroupTokenFilter::testCamelCase() 
{
  testPatterns(L"letsPartyLIKEits1999_dude",
               std::deque<wstring>{
                   L"([A-Z]{2,})", L"(?<![A-Z])([A-Z][a-z]+)",
                   L"(?:^|\\b|(?<=[0-9_])|(?<=[A-Z]{2}))([a-z]+)", L"([0-9]+)"},
               std::deque<wstring>{L"lets", L"Party", L"LIKE", L"its", L"1999",
                                    L"dude"},
               std::deque<int>{0, 0, 0, 0, 0, 0},
               std::deque<int>{25, 25, 25, 25, 25, 25},
               std::deque<int>{1, 0, 0, 0, 0, 0, 0}, false);
  testPatterns(
      L"letsPartyLIKEits1999_dude",
      std::deque<wstring>{L"([A-Z]{2,})", L"(?<![A-Z])([A-Z][a-z]+)",
                           L"(?:^|\\b|(?<=[0-9_])|(?<=[A-Z]{2}))([a-z]+)",
                           L"([0-9]+)"},
      std::deque<wstring>{L"letsPartyLIKEits1999_dude", L"lets", L"Party",
                           L"LIKE", L"its", L"1999", L"dude"},
      std::deque<int>{0, 0, 0, 0, 0, 0, 0},
      std::deque<int>{25, 25, 25, 25, 25, 25, 25},
      std::deque<int>{1, 0, 0, 0, 0, 0, 0, 0}, true);
}

void TestPatternCaptureGroupTokenFilter::testRandomString() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());

  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestPatternCaptureGroupTokenFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestPatternCaptureGroupTokenFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents> TestPatternCaptureGroupTokenFilter::
    AnalyzerAnonymousInnerClass::createComponents(const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<PatternCaptureGroupTokenFilter>(
                     tokenizer, false, Pattern::compile(L"((..)(..))")));
}

void TestPatternCaptureGroupTokenFilter::testPatterns(
    const wstring &input, std::deque<wstring> &regexes,
    std::deque<wstring> &tokens, std::deque<int> &startOffsets,
    std::deque<int> &endOffsets, std::deque<int> &positions,
    bool preserveOriginal) 
{
  std::deque<std::shared_ptr<Pattern>> patterns(regexes.size());
  for (int i = 0; i < regexes.size(); i++) {
    patterns[i] = Pattern::compile(regexes[i]);
  }

  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(input));
  shared_ptr<TokenStream> ts = make_shared<PatternCaptureGroupTokenFilter>(
      tokenizer, preserveOriginal, patterns);
  assertTokenStreamContents(ts, tokens, startOffsets, endOffsets, positions);
}
} // namespace org::apache::lucene::analysis::pattern