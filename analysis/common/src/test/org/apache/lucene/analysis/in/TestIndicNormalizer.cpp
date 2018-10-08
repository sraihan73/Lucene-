using namespace std;

#include "TestIndicNormalizer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/in/IndicNormalizationFilter.h"

namespace org::apache::lucene::analysis::in_
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using TokenFilter = org::apache::lucene::analysis::TokenFilter;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestIndicNormalizer::testBasics() 
{
  check(L"अाॅअाॅ", L"ऑऑ");
  check(L"अाॆअाॆ", L"ऒऒ");
  check(L"अाेअाे", L"ओओ");
  check(L"अाैअाै", L"औऔ");
  check(L"अाअा", L"आआ");
  check(L"अाैर", L"और");
  // khanda-ta
  check(L"ত্‍", L"ৎ");
}

void TestIndicNormalizer::check(const wstring &input,
                                const wstring &output) 
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::WHITESPACE, false);
  tokenizer->setReader(make_shared<StringReader>(input));
  shared_ptr<TokenFilter> tf = make_shared<IndicNormalizationFilter>(tokenizer);
  assertTokenStreamContents(tf, std::deque<wstring>{output});
}

void TestIndicNormalizer::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestIndicNormalizer::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestIndicNormalizer> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIndicNormalizer::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<IndicNormalizationFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::in_