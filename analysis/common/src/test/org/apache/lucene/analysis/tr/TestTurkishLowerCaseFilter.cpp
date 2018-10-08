using namespace std;

#include "TestTurkishLowerCaseFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/tr/TurkishLowerCaseFilter.h"

namespace org::apache::lucene::analysis::tr
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestTurkishLowerCaseFilter::testTurkishLowerCaseFilter() throw(
    runtime_error)
{
  shared_ptr<TokenStream> stream =
      whitespaceMockTokenizer(L"\u0130STANBUL \u0130ZM\u0130R ISPARTA");
  shared_ptr<TurkishLowerCaseFilter> filter =
      make_shared<TurkishLowerCaseFilter>(stream);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"istanbul", L"izmir", L"\u0131sparta"});
}

void TestTurkishLowerCaseFilter::testDecomposed() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(
      L"\u0049\u0307STANBUL \u0049\u0307ZM\u0049\u0307R ISPARTA");
  shared_ptr<TurkishLowerCaseFilter> filter =
      make_shared<TurkishLowerCaseFilter>(stream);
  assertTokenStreamContents(
      filter, std::deque<wstring>{L"istanbul", L"izmir", L"\u0131sparta"});
}

void TestTurkishLowerCaseFilter::testDecomposed2() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(
      L"\u0049\u0316\u0307STANBUL \u0049\u0307ZM\u0049\u0307R I\u0316SPARTA");
  shared_ptr<TurkishLowerCaseFilter> filter =
      make_shared<TurkishLowerCaseFilter>(stream);
  assertTokenStreamContents(
      filter,
      std::deque<wstring>{L"i\u0316stanbul", L"izmir", L"\u0131\u0316sparta"});
}

void TestTurkishLowerCaseFilter::testDecomposed3() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(L"\u0049\u0307");
  shared_ptr<TurkishLowerCaseFilter> filter =
      make_shared<TurkishLowerCaseFilter>(stream);
  assertTokenStreamContents(filter, std::deque<wstring>{L"i"});
}

void TestTurkishLowerCaseFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestTurkishLowerCaseFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestTurkishLowerCaseFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTurkishLowerCaseFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<TurkishLowerCaseFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::tr