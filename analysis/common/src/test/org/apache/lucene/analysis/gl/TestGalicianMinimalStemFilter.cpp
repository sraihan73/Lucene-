using namespace std;

#include "TestGalicianMinimalStemFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/CharArraySet.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/gl/GalicianMinimalStemFilter.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/SetKeywordMarkerFilter.h"

namespace org::apache::lucene::analysis::gl
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CharArraySet = org::apache::lucene::analysis::CharArraySet;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using SetKeywordMarkerFilter =
    org::apache::lucene::analysis::miscellaneous::SetKeywordMarkerFilter;

void TestGalicianMinimalStemFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  a = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestGalicianMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<GalicianMinimalStemFilter>(tokenizer));
}

void TestGalicianMinimalStemFilter::tearDown() 
{
  delete a;
  BaseTokenStreamTestCase::tearDown();
}

void TestGalicianMinimalStemFilter::testPlural() 
{
  checkOneTerm(a, L"elefantes", L"elefante");
  checkOneTerm(a, L"elefante", L"elefante");
  checkOneTerm(a, L"kalóres", L"kalór");
  checkOneTerm(a, L"kalór", L"kalór");
}

void TestGalicianMinimalStemFilter::testExceptions() 
{
  checkOneTerm(a, L"mas", L"mas");
  checkOneTerm(a, L"barcelonês", L"barcelonês");
}

void TestGalicianMinimalStemFilter::testKeyword() 
{
  shared_ptr<CharArraySet> *const exclusionSet =
      make_shared<CharArraySet>(asSet({L"elefantes"}), false);
  shared_ptr<Analyzer> a = make_shared<AnalyzerAnonymousInnerClass2>(
      shared_from_this(), exclusionSet);
  checkOneTerm(a, L"elefantes", L"elefantes");
  delete a;
}

TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestGalicianMinimalStemFilter> outerInstance,
        shared_ptr<CharArraySet> exclusionSet)
{
  this->outerInstance = outerInstance;
  this->exclusionSet = exclusionSet;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> source =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> sink =
      make_shared<SetKeywordMarkerFilter>(source, exclusionSet);
  return make_shared<Analyzer::TokenStreamComponents>(
      source, make_shared<GalicianMinimalStemFilter>(sink));
}

void TestGalicianMinimalStemFilter::testRandomStrings() 
{
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
}

void TestGalicianMinimalStemFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass3>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass3::
    AnalyzerAnonymousInnerClass3(
        shared_ptr<TestGalicianMinimalStemFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestGalicianMinimalStemFilter::AnalyzerAnonymousInnerClass3::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<GalicianMinimalStemFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::gl