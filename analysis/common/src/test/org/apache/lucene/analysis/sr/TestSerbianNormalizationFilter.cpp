using namespace std;

#include "TestSerbianNormalizationFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/sr/SerbianNormalizationFilter.h"

namespace org::apache::lucene::analysis::sr
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestSerbianNormalizationFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestSerbianNormalizationFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestSerbianNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSerbianNormalizationFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> *const tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  shared_ptr<TokenStream> *const stream =
      make_shared<SerbianNormalizationFilter>(tokenizer);
  return make_shared<Analyzer::TokenStreamComponents>(tokenizer, stream);
}

void TestSerbianNormalizationFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestSerbianNormalizationFilter::testCyrillic() 
{
  checkOneTerm(analyzer, L"абвгдђежзијклљмнњопрстћуфхцчџш",
               L"abvgddjezzijklljmnnjoprstcufhccdzs");
}

void TestSerbianNormalizationFilter::testLatin() 
{
  checkOneTerm(analyzer, L"abcčćddžđefghijklljmnnjoprsštuvzž",
               L"abcccddzdjefghijklljmnnjoprsstuvzz");
}

void TestSerbianNormalizationFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}

void TestSerbianNormalizationFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestSerbianNormalizationFilter::AnalyzerAnonymousInnerClass2::
    AnalyzerAnonymousInnerClass2(
        shared_ptr<TestSerbianNormalizationFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestSerbianNormalizationFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<SerbianNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::sr