using namespace std;

#include "FixedShingleFilterTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/shingle/FixedShingleFilter.h"

namespace org::apache::lucene::analysis::shingle
{
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;

void FixedShingleFilterTest::testBiGramFilter() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"please", 0, 6), make_shared<Token>(L"divide", 7, 13),
      make_shared<Token>(L"this", 14, 18),
      make_shared<Token>(L"sentence", 19, 27),
      make_shared<Token>(L"into", 28, 32),
      make_shared<Token>(L"shingles", 33, 41));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 2),
      std::deque<wstring>{L"please divide", L"divide this", L"this sentence",
                           L"sentence into", L"into shingles"},
      std::deque<int>{0, 7, 14, 19, 28}, std::deque<int>{13, 18, 27, 32, 41},
      std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                           L"shingle"},
      std::deque<int>{1, 1, 1, 1, 1}, std::deque<int>{1, 1, 1, 1, 1});
}

void FixedShingleFilterTest::testBiGramFilterWithAltSeparator() throw(
    IOException)
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"please", 0, 6), make_shared<Token>(L"divide", 7, 13),
      make_shared<Token>(L"this", 14, 18),
      make_shared<Token>(L"sentence", 19, 27),
      make_shared<Token>(L"into", 28, 32),
      make_shared<Token>(L"shingles", 33, 41));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 2, L"<SEP>", L"_"),
      std::deque<wstring>{L"please<SEP>divide", L"divide<SEP>this",
                           L"this<SEP>sentence", L"sentence<SEP>into",
                           L"into<SEP>shingles"},
      std::deque<int>{0, 7, 14, 19, 28}, std::deque<int>{13, 18, 27, 32, 41},
      std::deque<wstring>{L"shingle", L"shingle", L"shingle", L"shingle",
                           L"shingle"},
      std::deque<int>{1, 1, 1, 1, 1});
}

void FixedShingleFilterTest::testTriGramFilter() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"please", 0, 6), make_shared<Token>(L"divide", 7, 13),
      make_shared<Token>(L"this", 14, 18),
      make_shared<Token>(L"sentence", 19, 27),
      make_shared<Token>(L"into", 28, 32),
      make_shared<Token>(L"shingles", 33, 41));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 3),
      std::deque<wstring>{L"please divide this", L"divide this sentence",
                           L"this sentence into", L"sentence into shingles"});
}

void FixedShingleFilterTest::
    testShingleSizeGreaterThanTokenstreamLength() 
{

  shared_ptr<TokenStream> ts = make_shared<FixedShingleFilter>(
      make_shared<CannedTokenStream>(make_shared<Token>(L"please", 0, 6),
                                     make_shared<Token>(L"divide", 7, 13)),
      3);

  ts->reset();
  assertFalse(ts->incrementToken());
}

void FixedShingleFilterTest::testWithStopwords() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"please", 0, 6), make_shared<Token>(L"divide", 7, 13),
      make_shared<Token>(L"sentence", 2, 19, 27),
      make_shared<Token>(L"shingles", 2, 33, 41));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 3),
      std::deque<wstring>{L"please divide _", L"divide _ sentence",
                           L"sentence _ shingles"},
      std::deque<int>{0, 7, 19}, std::deque<int>{13, 27, 41},
      std::deque<wstring>{L"shingle", L"shingle", L"shingle"},
      std::deque<int>{1, 1, 2});
}

void FixedShingleFilterTest::testConsecutiveStopwords() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"b", 2, 2, 3), make_shared<Token>(L"c", 4, 5),
      make_shared<Token>(L"d", 6, 7), make_shared<Token>(L"b", 3, 12, 13),
      make_shared<Token>(L"c", 14, 15));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 4),
      std::deque<wstring>{L"b c d _", L"c d _ _", L"d _ _ b"},
      std::deque<int>{2, 4, 6}, std::deque<int>{7, 7, 13},
      std::deque<int>{2, 1, 1});
}

void FixedShingleFilterTest::testTrailingStopwords() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      1, 7, make_shared<Token>(L"b", 0, 1), make_shared<Token>(L"c", 2, 3),
      make_shared<Token>(L"d", 4, 5));

  assertTokenStreamContents(make_shared<FixedShingleFilter>(ts, 3),
                            std::deque<wstring>{L"b c d", L"c d _"},
                            std::deque<int>{0, 2}, std::deque<int>{5, 5},
                            std::deque<int>{1, 1});
}

void FixedShingleFilterTest::testMultipleTrailingStopwords() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      2, 9, make_shared<Token>(L"b", 0, 1), make_shared<Token>(L"c", 2, 3),
      make_shared<Token>(L"d", 4, 5));

  assertTokenStreamContents(make_shared<FixedShingleFilter>(ts, 3),
                            std::deque<wstring>{L"b c d", L"c d _", L"d _ _"},
                            std::deque<int>{0, 2, 4},
                            std::deque<int>{5, 5, 5},
                            std::deque<int>{1, 1, 1});
}

void FixedShingleFilterTest::testIncomingGraphs() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"b", 0, 1), make_shared<Token>(L"a", 0, 0, 1),
      make_shared<Token>(L"c", 2, 3), make_shared<Token>(L"b", 4, 5),
      make_shared<Token>(L"a", 0, 4, 5), make_shared<Token>(L"d", 6, 7));

  assertTokenStreamContents(
      make_shared<FixedShingleFilter>(ts, 2),
      std::deque<wstring>{L"b c", L"a c", L"c b", L"c a", L"b d", L"a d"},
      std::deque<int>{0, 0, 2, 2, 4, 4}, std::deque<int>{3, 3, 5, 5, 7, 7},
      std::deque<int>{1, 0, 1, 0, 1, 0});
}

void FixedShingleFilterTest::testShinglesSpanningGraphs() 
{

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(L"b", 0, 1), make_shared<Token>(L"a", 0, 0, 1),
      make_shared<Token>(L"c", 2, 3), make_shared<Token>(L"b", 4, 5),
      make_shared<Token>(L"a", 0, 4, 5), make_shared<Token>(L"d", 6, 7));

  assertTokenStreamContents(make_shared<FixedShingleFilter>(ts, 3),
                            std::deque<wstring>{L"b c b", L"b c a", L"a c b",
                                                 L"a c a", L"c b d", L"c a d"},
                            std::deque<int>{0, 0, 0, 0, 2, 2},
                            std::deque<int>{5, 5, 5, 5, 7, 7},
                            std::deque<int>{1, 0, 0, 0, 1, 0});
}

void FixedShingleFilterTest::testParameterLimits()
{
  invalid_argument e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FixedShingleFilter>(make_shared<CannedTokenStream>(), 1);
  });
  assertEquals(L"Shingle size must be between 2 and 4, got 1", e.what());
  invalid_argument e2 = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<FixedShingleFilter>(make_shared<CannedTokenStream>(), 5);
  });
  assertEquals(L"Shingle size must be between 2 and 4, got 5", e2.what());
}

void FixedShingleFilterTest::testShingleCountLimits()
{

  std::deque<std::shared_ptr<Token>> tokens(5000);
  tokens[0] = make_shared<Token>(L"term", 1, 0, 1);
  tokens[1] = make_shared<Token>(L"term1", 1, 2, 3);
  for (int i = 2; i < 5000; i++) {
    tokens[i] = make_shared<Token>(L"term" + to_wstring(i), 0, 2, 3);
  }

  runtime_error e = expectThrows(IllegalStateException::typeid, [&]() {
    shared_ptr<TokenStream> ts = make_shared<FixedShingleFilter>(
        make_shared<CannedTokenStream>(tokens), 2);
    ts->reset();
    while (ts->incrementToken()) {
    }
  });
  assertEquals(L"Too many shingles (> 1000) at term [term]", e.what());
}
} // namespace org::apache::lucene::analysis::shingle