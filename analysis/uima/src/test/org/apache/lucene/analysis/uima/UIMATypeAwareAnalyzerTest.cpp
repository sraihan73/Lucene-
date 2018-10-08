using namespace std;

#include "UIMATypeAwareAnalyzerTest.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/Analyzer.h"
#include "../../../../../../../../../core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include "../../../../../../java/org/apache/lucene/analysis/uima/UIMATypeAwareAnalyzer.h"

namespace org::apache::lucene::analysis::uima
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using TokenStream = org::apache::lucene::analysis::TokenStream;
using org::junit::After;
using org::junit::Before;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @Before public void setUp() throws Exception
void UIMATypeAwareAnalyzerTest::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<UIMATypeAwareAnalyzer>(
      L"/uima/AggregateSentenceAE.xml", L"org.apache.uima.TokenAnnotation",
      L"posTag", nullptr);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Override @After public void tearDown() throws Exception
void UIMATypeAwareAnalyzerTest::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void baseUIMATypeAwareAnalyzerStreamTest() throws
// Exception
void UIMATypeAwareAnalyzerTest::baseUIMATypeAwareAnalyzerStreamTest() throw(
    runtime_error)
{

  // create a token stream
  shared_ptr<TokenStream> ts =
      analyzer->tokenStream(L"text", L"the big brown fox jumped on the wood");

  // check that 'the big brown fox jumped on the wood' tokens have the expected
  // PoS types
  assertTokenStreamContents(ts,
                            std::deque<wstring>{L"the", L"big", L"brown",
                                                 L"fox", L"jumped", L"on",
                                                 L"the", L"wood"},
                            std::deque<wstring>{L"at", L"jj", L"jj", L"nn",
                                                 L"vbd", L"in", L"at", L"nn"});
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test @AwaitsFix(bugUrl =
// "https://issues.apache.org/jira/browse/LUCENE-3869") public void
// testRandomStrings() throws Exception
void UIMATypeAwareAnalyzerTest::testRandomStrings() 
{
  shared_ptr<Analyzer> analyzer = make_shared<UIMATypeAwareAnalyzer>(
      L"/uima/TestAggregateSentenceAE.xml",
      L"org.apache.lucene.uima.ts.TokenAnnotation", L"pos", nullptr);
  checkRandomData(random(), analyzer, 100 * RANDOM_MULTIPLIER);
  delete analyzer;
}
} // namespace org::apache::lucene::analysis::uima