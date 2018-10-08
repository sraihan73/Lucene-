using namespace std;

#include "TestGermanNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/de/GermanNormalizationFilter.h"

namespace org::apache::lucene::analysis::de
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestGermanNormalizationFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestGermanNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestGermanNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGermanNormalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &field)
{
  shared_ptr<Tokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const stream =
      make_shared<GermanNormalizationFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestGermanNormalizationFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestGermanNormalizationFilter::testBasicExamples() 
{
  checkOneTerm(analyzer, L"Schaltflächen", L"Schaltflachen");
  checkOneTerm(analyzer, L"Schaltflaechen", L"Schaltflachen");
}

void TestGermanNormalizationFilter::testUHeuristic() 
{
  checkOneTerm(analyzer, L"dauer", L"dauer");
}

void TestGermanNormalizationFilter::testSpecialFolding() 
{
  checkOneTerm(analyzer, L"weißbier", L"weissbier");
}

void TestGermanNormalizationFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestGermanNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestGermanNormalizationFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestGermanNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGermanNormalizationFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<GermanNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::de