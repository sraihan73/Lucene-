using namespace std;

#include "TestCJKBigramFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/standard/StandardTokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../java/org/apache/lucene/analysis/cjk/CJKBigramFilter.h"

namespace org::apache::lucene::analysis::cjk
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using StandardTokenizer =
    org::apache::lucene::analysis::standard::StandardTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestCJKBigramFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  unibiAnalyzer = make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
}

TestCJKBigramFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKBigramFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<CJKBigramFilter>(t));
}

TestCJKBigramFilter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKBigramFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<CJKBigramFilter>(t, 0xff, true));
}

void TestCJKBigramFilter::tearDown() 
{
  IOUtils::close({analyzer, unibiAnalyzer});
  BaseTokenStreamTestCase::tearDown();
}

void TestCJKBigramFilter::testHuge() 
{
  assertAnalyzesTo(
      analyzer,
      wstring(L"多くの学生が試験に落ちた") + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた",
      std::deque<wstring>{
          L"多く", L"くの", L"の学", L"学生", L"生が", L"が試", L"試験",
          L"験に", L"に落", L"落ち", L"ちた", L"た多", L"多く", L"くの",
          L"の学", L"学生", L"生が", L"が試", L"試験", L"験に", L"に落",
          L"落ち", L"ちた", L"た多", L"多く", L"くの", L"の学", L"学生",
          L"生が", L"が試", L"試験", L"験に", L"に落", L"落ち", L"ちた",
          L"た多", L"多く", L"くの", L"の学", L"学生", L"生が", L"が試",
          L"試験", L"験に", L"に落", L"落ち", L"ちた", L"た多", L"多く",
          L"くの", L"の学", L"学生", L"生が", L"が試", L"試験", L"験に",
          L"に落", L"落ち", L"ちた", L"た多", L"多く", L"くの", L"の学",
          L"学生", L"生が", L"が試", L"試験", L"験に", L"に落", L"落ち",
          L"ちた", L"た多", L"多く", L"くの", L"の学", L"学生", L"生が",
          L"が試", L"試験", L"験に", L"に落", L"落ち", L"ちた", L"た多",
          L"多く", L"くの", L"の学", L"学生", L"生が", L"が試", L"試験",
          L"験に", L"に落", L"落ち", L"ちた", L"た多", L"多く", L"くの",
          L"の学", L"学生", L"生が", L"が試", L"試験", L"験に", L"に落",
          L"落ち", L"ちた", L"た多", L"多く", L"くの", L"の学", L"学生",
          L"生が", L"が試", L"試験", L"験に", L"に落", L"落ち", L"ちた",
          L"た多", L"多く", L"くの", L"の学", L"学生", L"生が", L"が試",
          L"試験", L"験に", L"に落", L"落ち", L"ちた"});
}

void TestCJKBigramFilter::testHanOnly() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  assertAnalyzesTo(a, L"多くの学生が試験に落ちた。",
                   std::deque<wstring>{L"多", L"く", L"の", L"学生", L"が",
                                        L"試験", L"に", L"落", L"ち", L"た"},
                   std::deque<int>{0, 1, 2, 3, 5, 6, 8, 9, 10, 11},
                   std::deque<int>{1, 2, 3, 5, 6, 8, 9, 10, 11, 12},
                   std::deque<wstring>{
                       L"<SINGLE>", L"<HIRAGANA>", L"<HIRAGANA>", L"<DOUBLE>",
                       L"<HIRAGANA>", L"<DOUBLE>", L"<HIRAGANA>", L"<SINGLE>",
                       L"<HIRAGANA>", L"<HIRAGANA>", L"<SINGLE>"},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
                   std::deque<int>{1, 1, 1, 1, 1, 1, 1, 1, 1, 1});
  delete a;
}

TestCJKBigramFilter::AnalyzerAnonymousInnerClass3::AnalyzerAnonymousInnerClass3(
    shared_ptr<TestCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKBigramFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<CJKBigramFilter>(t, CJKBigramFilter::HAN));
}

void TestCJKBigramFilter::testAllScripts() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass4>(shared_from_this());
  assertAnalyzesTo(a, L"多くの学生が試験に落ちた。",
                   std::deque<wstring>{L"多く", L"くの", L"の学", L"学生",
                                        L"生が", L"が試", L"試験", L"験に",
                                        L"に落", L"落ち", L"ちた"});
  delete a;
}

TestCJKBigramFilter::AnalyzerAnonymousInnerClass4::AnalyzerAnonymousInnerClass4(
    shared_ptr<TestCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKBigramFilter::AnalyzerAnonymousInnerClass4::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<CJKBigramFilter>(t, 0xff, false));
}

void TestCJKBigramFilter::testUnigramsAndBigramsAllScripts() throw(
    runtime_error)
{
  assertAnalyzesTo(
      unibiAnalyzer, L"多くの学生が試験に落ちた。",
      std::deque<wstring>{L"多", L"多く", L"く", L"くの", L"の", L"の学",
                           L"学", L"学生", L"生", L"生が", L"が", L"が試",
                           L"試", L"試験", L"験", L"験に", L"に", L"に落",
                           L"落", L"落ち", L"ち", L"ちた", L"た"},
      std::deque<int>{0, 0, 1, 1, 2, 2, 3, 3, 4,  4,  5, 5,
                       6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11},
      std::deque<int>{1, 2, 2, 3, 3, 4,  4,  5,  5,  6,  6, 7,
                       7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12},
      std::deque<wstring>{L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<DOUBLE>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<DOUBLE>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<DOUBLE>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<DOUBLE>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<DOUBLE>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>"},
      std::deque<int>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
                       1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1},
      std::deque<int>{1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
                       1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1});
}

void TestCJKBigramFilter::testUnigramsAndBigramsHanOnly() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass5>(shared_from_this());
  assertAnalyzesTo(
      a, L"多くの学生が試験に落ちた。",
      std::deque<wstring>{L"多", L"く", L"の", L"学", L"学生", L"生", L"が",
                           L"試", L"試験", L"験", L"に", L"落", L"ち", L"た"},
      std::deque<int>{0, 1, 2, 3, 3, 4, 5, 6, 6, 7, 8, 9, 10, 11},
      std::deque<int>{1, 2, 3, 4, 5, 5, 6, 7, 8, 8, 9, 10, 11, 12},
      std::deque<wstring>{L"<SINGLE>", L"<HIRAGANA>", L"<HIRAGANA>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<HIRAGANA>",
                           L"<SINGLE>", L"<DOUBLE>", L"<SINGLE>", L"<HIRAGANA>",
                           L"<SINGLE>", L"<HIRAGANA>", L"<HIRAGANA>",
                           L"<SINGLE>"},
      std::deque<int>{1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1},
      std::deque<int>{1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1});
  delete a;
}

TestCJKBigramFilter::AnalyzerAnonymousInnerClass5::AnalyzerAnonymousInnerClass5(
    shared_ptr<TestCJKBigramFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCJKBigramFilter::AnalyzerAnonymousInnerClass5::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> t = make_shared<StandardTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      t, make_shared<CJKBigramFilter>(t, CJKBigramFilter::HAN, true));
}

void TestCJKBigramFilter::testUnigramsAndBigramsHuge() 
{
  assertAnalyzesTo(
      unibiAnalyzer,
      wstring(L"多くの学生が試験に落ちた") + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた" + L"多くの学生が試験に落ちた" +
          L"多くの学生が試験に落ちた",
      std::deque<wstring>{
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た", L"た多",
          L"多", L"多く", L"く", L"くの", L"の", L"の学", L"学", L"学生",
          L"生", L"生が", L"が", L"が試", L"試", L"試験", L"験", L"験に",
          L"に", L"に落", L"落", L"落ち", L"ち", L"ちた", L"た"});
}

void TestCJKBigramFilter::testRandomUnibiStrings() 
{
  checkRandomData(random(), unibiAnalyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestCJKBigramFilter::testRandomUnibiHugeStrings() 
{
  shared_ptr<Random> random = TestCJKBigramFilter::random();
  checkRandomData(random, unibiAnalyzer, 100 * RANDOM_MULTIPLIER, 8192);
}
} // namespace org::apache::lucene::analysis::cjk