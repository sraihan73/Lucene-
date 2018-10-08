using namespace std;

#include "TestPersianCharFilter.h"
#include "../../../../../../../../../test-framework/src/java/org/apache/lucene/analysis/MockTokenizer.h"
#include "../../../../../../java/org/apache/lucene/analysis/fa/PersianCharFilter.h"

namespace org::apache::lucene::analysis::fa
{
using Analyzer = org::apache::lucene::analysis::Analyzer;
using BaseTokenStreamTestCase =
    org::apache::lucene::analysis::BaseTokenStreamTestCase;
using MockTokenizer = org::apache::lucene::analysis::MockTokenizer;

void TestPersianCharFilter::setUp() 
{
  BaseTokenStreamTestCase::setUp();
  analyzer = make_shared<AnalyzerAnonymousInnerClass>(shared_from_this());
}

TestPersianCharFilter::AnalyzerAnonymousInnerClass::AnalyzerAnonymousInnerClass(
    shared_ptr<TestPersianCharFilter> outerInstance)
{
  this->outerInstance = outerInstance;
}

shared_ptr<Analyzer::TokenStreamComponents>
TestPersianCharFilter::AnalyzerAnonymousInnerClass::createComponents(
    const wstring &fieldName)
{
  return make_shared<Analyzer::TokenStreamComponents>(
      make_shared<MockTokenizer>());
}

shared_ptr<Reader>
TestPersianCharFilter::AnalyzerAnonymousInnerClass::initReader(
    const wstring &fieldName, shared_ptr<Reader> reader)
{
  return make_shared<PersianCharFilter>(reader);
}

void TestPersianCharFilter::tearDown() 
{
  delete analyzer;
  BaseTokenStreamTestCase::tearDown();
}

void TestPersianCharFilter::testBasics() 
{
  assertAnalyzesTo(analyzer, L"this is a\u200Ctest",
                   std::deque<wstring>{L"this", L"is", L"a", L"test"});
}

void TestPersianCharFilter::testRandomStrings() 
{
  checkRandomData(random(), analyzer, 1000 * RANDOM_MULTIPLIER);
}
} // namespace org::apache::lucene::analysis::fa