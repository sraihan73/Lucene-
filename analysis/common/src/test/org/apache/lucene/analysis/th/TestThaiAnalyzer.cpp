using namespace std;

#include "TestThaiAnalyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/tokenattributes/FlagsAttribute.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/StopAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/th/ThaiAnalyzer.h"
#include "../../../../../../java/org/apache/lucene/analysis/th/ThaiTokenizer.h"

namespace org::apache::lucene::analysis::th
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using StopAnalyzer = org::apache::lucene::analysis::core::StopAnalyzer;
using FlagsAttribute =
    org::apache::lucene::analysis::tokenattributes::FlagsAttribute;

void TestThaiAnalyzer::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  assumeTrue(L"JRE does not support Thai dictionary-based BreakIterator",
             ThaiTokenizer::DBBI_AVAILABLE);
}

void TestThaiAnalyzer::testOffsets() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<ThaiAnalyzer>(CharArraySet::EMPTY_SET);
  assertAnalyzesTo(analyzer, L"การที่ได้ต้องแสดงว่างานดี",
                   std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง",
                                        L"ว่า", L"งาน", L"ดี"},
                   std::deque<int>{0, 3, 6, 9, 13, 17, 20, 23},
                   std::deque<int>{3, 6, 9, 13, 17, 20, 23, 25});
  delete analyzer;
}

void TestThaiAnalyzer::testStopWords() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ThaiAnalyzer>();
  assertAnalyzesTo(analyzer, L"การที่ได้ต้องแสดงว่างานดี",
                   std::deque<wstring>{L"แสดง", L"งาน", L"ดี"},
                   std::deque<int>{13, 20, 23}, std::deque<int>{17, 23, 25},
                   std::deque<int>{5, 2, 1});
  delete analyzer;
}

void TestThaiAnalyzer::testPositionIncrements() 
{
  shared_ptr<ThaiAnalyzer> *const analyzer =
      make_shared<ThaiAnalyzer>(StopAnalyzer::ENGLISH_STOP_WORDS_SET);
  assertAnalyzesTo(analyzer, L"การที่ได้ต้อง the แสดงว่างานดี",
                   std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง",
                                        L"ว่า", L"งาน", L"ดี"},
                   std::deque<int>{0, 3, 6, 9, 18, 22, 25, 28},
                   std::deque<int>{3, 6, 9, 13, 22, 25, 28, 30},
                   std::deque<int>{1, 1, 1, 1, 2, 1, 1, 1});

  // case that a stopword is adjacent to thai text, with no whitespace
  assertAnalyzesTo(analyzer, L"การที่ได้ต้องthe แสดงว่างานดี",
                   std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง",
                                        L"ว่า", L"งาน", L"ดี"},
                   std::deque<int>{0, 3, 6, 9, 17, 21, 24, 27},
                   std::deque<int>{3, 6, 9, 13, 21, 24, 27, 29},
                   std::deque<int>{1, 1, 1, 1, 2, 1, 1, 1});
  delete analyzer;
}

void TestThaiAnalyzer::testReusableTokenStream() 
{
  shared_ptr<ThaiAnalyzer> analyzer =
      make_shared<ThaiAnalyzer>(CharArraySet::EMPTY_SET);
  assertAnalyzesTo(analyzer, L"", std::deque<wstring>());

  assertAnalyzesTo(analyzer, L"การที่ได้ต้องแสดงว่างานดี",
                   std::deque<wstring>{L"การ", L"ที่", L"ได้", L"ต้อง", L"แสดง",
                                        L"ว่า", L"งาน", L"ดี"});

  assertAnalyzesTo(analyzer, L"บริษัทชื่อ XY&Z - คุยกับ xyz@demo.com",
                   std::deque<wstring>{L"บริษัท", L"ชื่อ", L"xy", L"z", L"คุย",
                                        L"กับ", L"xyz", L"demo.com"});
  delete analyzer;
}

void TestThaiAnalyzer::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ThaiAnalyzer>();
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
  delete analyzer;
}

void TestThaiAnalyzer::testRandomHugeStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<ThaiAnalyzer>();
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER, 8192);
  delete analyzer;
}

void TestThaiAnalyzer::testAttributeReuse() 
{
  shared_ptr<ThaiAnalyzer> analyzer = make_shared<ThaiAnalyzer>();
  // just consume
  shared_ptr<TokenStream> ts = analyzer->tokenStream(L"dummy", L"ภาษาไทย");
  assertTokenStreamContents(ts, std::deque<wstring>{L"ภาษา", L"ไทย"});
  // this consumer adds flagsAtt, which this analyzer does not use.
  ts = analyzer->tokenStream(L"dummy", L"ภาษาไทย");
  ts->addAttribute(FlagsAttribute::typeid);
  assertTokenStreamContents(ts, std::deque<wstring>{L"ภาษา", L"ไทย"});
  delete analyzer;
}

void TestThaiAnalyzer::testDigits() 
{
  shared_ptr<ThaiAnalyzer> a = make_shared<ThaiAnalyzer>();
  checkOneTerm(a, L"๑๒๓๔", L"1234");
  delete a;
}

void TestThaiAnalyzer::testTwoSentences() 
{
  shared_ptr<Analyzer> analyzer =
      make_shared<ThaiAnalyzer>(CharArraySet::EMPTY_SET);
  assertAnalyzesTo(
      analyzer, L"This is a test. การที่ได้ต้องแสดงว่างานดี",
      std::deque<wstring>{L"this", L"is", L"a", L"test", L"การ", L"ที่", L"ได้",
                           L"ต้อง", L"แสดง", L"ว่า", L"งาน", L"ดี"},
      std::deque<int>{0, 5, 8, 10, 16, 19, 22, 25, 29, 33, 36, 39},
      std::deque<int>{4, 7, 9, 14, 19, 22, 25, 29, 33, 36, 39, 41});
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::th