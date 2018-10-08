using namespace std;

#include "TestTrimFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/CannedTokenStream.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/Token.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/TrimFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using CannedTokenStream = org::apache::lucene::analysis::CannedTokenStream;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;
using Token = org::apache::lucene::analysis::Token;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;

void TestTrimFilter::testTrim() 
{
  std::deque<wchar_t> a = (wstring(L" a ")).toCharArray();
  std::deque<wchar_t> b = (wstring(L"b   ")).toCharArray();
  std::deque<wchar_t> ccc = (wstring(L"cCc")).toCharArray();
  std::deque<wchar_t> whitespace = (wstring(L"   ")).toCharArray();
  std::deque<wchar_t> empty = (wstring(L"")).toCharArray();

  shared_ptr<TokenStream> ts = make_shared<CannedTokenStream>(
      make_shared<Token>(wstring(a, 0, a.size()), 1, 5),
      make_shared<Token>(wstring(b, 0, b.size()), 6, 10),
      make_shared<Token>(wstring(ccc, 0, ccc.size()), 11, 15),
      make_shared<Token>(wstring(whitespace, 0, whitespace.size()), 16, 20),
      make_shared<Token>(wstring(empty, 0, empty.size()), 21, 21));
  ts = make_shared<TrimFilter>(ts);

  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"a", L"b", L"cCc", L"", L""});
}

void TestTrimFilter::testRandomStrings() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkRandomData(random(), a, 1000 * RANDOM_MULTIPLIER);
  delete a;
}

TestTrimFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestTrimFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTrimFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer =
      make_shared<MockTokenizer>(MockTokenizer::KEYWORD, false);
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<TrimFilter>(tokenizer));
}

void TestTrimFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass2>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestTrimFilter::AnalyzerAnonymousInnerClass2::AnalyzerAnonymousInnerClass2(
    shared_ptr<TestTrimFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestTrimFilter::AnalyzerAnonymousInnerClass2::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<TrimFilter>(tokenizer));
}
} // namespace org::apache::lucene::analysis::miscellaneous