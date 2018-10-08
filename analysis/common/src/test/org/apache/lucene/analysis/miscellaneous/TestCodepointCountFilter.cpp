using namespace std;

#include "TestCodepointCountFilter.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Tokenizer.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/util/TestUtil.h"
#include "../../../../../../java/org/apache/lucene/analysis/core/KeywordTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/miscellaneous/CodepointCountFilter.h"

namespace org::apache::lucene::analysis::miscellaneous
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using Tokenizer = org::apache::lucene::analysis::Tokenizer;
using KeywordTokenizer = org::apache::lucene::analysis::core::KeywordTokenizer;
using TestUtil = org::apache::lucene::util::TestUtil;
using org::junit::Test;

void TestCodepointCountFilter::testFilterWithPosIncr() 
{
  shared_ptr<TokenStream> stream = whitespaceMockTokenizer(
      L"short toolong evenmuchlongertext a ab toolong foo");
  shared_ptr<CodepointCountFilter> filter =
      make_shared<CodepointCountFilter>(stream, 2, 6);
  assertTokenStreamContents(filter,
                            std::deque<wstring>{L"short", L"ab", L"foo"},
                            std::deque<int>{1, 4, 2});
}

void TestCodepointCountFilter::testEmptyTerm() 
{
  shared_ptr<Analyzer> a =
      make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
  checkOneTerm(a, L"", L"");
  delete a;
}

TestCodepointCountFilter::AnalyzerAnonymousInnerClass::
    AnalyzerAnonymousInnerClass(
        shared_ptr<TestCodepointCountFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestCodepointCountFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  shared_ptr<Tokenizer> tokenizer = make_shared<KeywordTokenizer>();
  return make_shared<Analyzer::TokenStreamComponents>(
      tokenizer, make_shared<CodepointCountFilter>(tokenizer, 0, 5));
}

void TestCodepointCountFilter::testRandomStrings() 
{
  for (int i = 0; i < 10000; i++) {
    wstring text = TestUtil::randomUnicodeString(random(), 100);
    int min = TestUtil::nextInt(random(), 0, 100);
    int max = TestUtil::nextInt(random(), 0, 100);
    int count = text.codePointCount(0, text.length());
    if (min > max) {
      int temp = min;
      min = max;
      max = temp;
    }
    bool expected = count >= min && count <= max;
    shared_ptr<TokenStream> stream = make_shared<KeywordTokenizer>();
    (std::static_pointer_cast<Tokenizer>(stream))
        ->setReader(make_shared<StringReader>(text));
    stream = make_shared<CodepointCountFilter>(stream, min, max);
    stream->reset();
    TestUtil::assertEquals(expected, stream->incrementToken());
    stream->end();
    delete stream;
  }
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test(expected = IllegalArgumentException.class) public void
// testIllegalArguments() throws Exception
void TestCodepointCountFilter::testIllegalArguments() 
{
  make_shared<CodepointCountFilter>(
      whitespaceMockTokenizer(L"accept only valid arguments"), 4, 1);
}
} // namespace org::apache::lucene::analysis::miscellaneous