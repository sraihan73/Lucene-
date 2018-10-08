using namespace std;

#include "TestKoreanReadingFormFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/util/IOUtils.h"
#include "../../../../../../../../common/src/java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanReadingFormFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/ko/KoreanTokenizer.h"

namespace org::apache::lucene::analysis::ko
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using IOUtils = org::apache::lucene::util::IOUtils;

void TestKoreanReadingFormFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestKoreanReadingFormFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestKoreanReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanReadingFormFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KoreanTokenizer>(
      BaseTokenStreamTestCase::newAttributeFactory(), nullptr,
      KoreanTokenizer::DecompoundMode::DISCARD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<KoreanReadingFormFilter>(tokenizer));
}

void TestKoreanReadingFormFilter::tearDown() 
{
  IOUtils::close({analyzer});
  BaseTokenStreamTestCase::tearDown();
}

void TestKoreanReadingFormFilter::testReadings() 
{
  assertAnalyzesTo(analyzer, L"車丞相", std::deque<wstring>{L"차", L"승상"});
}

void TestKoreanReadingFormFilter::testRandomData() 
{
  shared_ptr<Random> random = TestKoreanReadingFormFilter::random();
  checkRandomData(random, analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestKoreanReadingFormFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestKoreanReadingFormFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestKoreanReadingFormFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestKoreanReadingFormFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<KoreanReadingFormFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ko