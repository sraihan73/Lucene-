using namespace std;

#include "TestIrishLowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/ga/IrishLowerCaseFilter.h"

namespace org::apache::lucene::analysis::ga
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestIrishLowerCaseFilter::testIrishLowerCaseFilter() 
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"nAthair tUISCE hARD");
  shared_ptr<IrishLowerCaseFilter> filter =
      make_shared<IrishLowerCaseFilter>(stream);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"n-athair", L"t-uisce", L"hard"});
}

void TestIrishLowerCaseFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestIrishLowerCaseFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestIrishLowerCaseFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestIrishLowerCaseFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<IrishLowerCaseFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::ga