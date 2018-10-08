using namespace std;

#include "TestLengthFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/LengthFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using org::junit::Test;

void TestLengthFilter::testFilterWithPosIncr() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(
      L"short toolong evenmuchlongertext a ab toolong foo");
  shared_ptr<LengthFilter> filter = make_shared<LengthFilter>(stream, 2, 6);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"short", L"ab", L"foo"},
                            std::deque<int>{1, 4, 2});
}

void TestLengthFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestLengthFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestLengthFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestLengthFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<LengthFilter>(tokenizer, 0, 5));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments() throws Exception
void TestLengthFilter::testIllegalArguments() 
{
  make_shared<LengthFilter>(
      whitespaceMockTokenizer(L"accept only valid arguments"), -4, -1);
}
} // namespace org::apache::lucene::analysis::miscellaneous