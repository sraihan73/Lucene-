using namespace std;

#include "TestEnglishMinimalStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/en/EnglishMinimalStemFilter.h"

namespace org::apache::lucene::analysis::en
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestEnglishMinimalStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestEnglishMinimalStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestEnglishMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestEnglishMinimalStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<EnglishMinimalStemFilter>(source));
}

void TestEnglishMinimalStemFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestEnglishMinimalStemFilter::testExamples() 
{
  checkOneTerm(analyzer, L"queries", L"query");
  checkOneTerm(analyzer, L"phrases", L"phrase");
  checkOneTerm(analyzer, L"corpus", L"corpus");
  checkOneTerm(analyzer, L"stress", L"stress");
  checkOneTerm(analyzer, L"kings", L"king");
  checkOneTerm(analyzer, L"panels", L"panel");
  checkOneTerm(analyzer, L"aerodynamics", L"aerodynamic");
  checkOneTerm(analyzer, L"congress", L"congress");
  checkOneTerm(analyzer, L"serious", L"serious");
}

void TestEnglishMinimalStemFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestEnglishMinimalStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestEnglishMinimalStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestEnglishMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestEnglishMinimalStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<EnglishMinimalStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::en