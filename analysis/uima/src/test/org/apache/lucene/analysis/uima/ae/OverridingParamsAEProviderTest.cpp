using namespace std;

#include "OverridingParamsAEProviderTest.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/AEProvider.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/OverridingParamsAEProvider.h"

namespace org::apache::lucene::analysis::uima::ae
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;
using org::apache::uima::analysis_engine::AnalysisEngine;
using org::apache::uima::resource::ResourceInitializationException;
using org::junit::Test;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testNullMapInitialization() throws Exception
void OverridingParamsAEProviderTest::testNullMapInitialization() throw(
    runtime_error)
{
  expectThrows(ResourceInitializationException::typeid, [&]() {
    shared_ptr<AEProvider> aeProvider = make_shared<OverridingParamsAEProvider>(
        L"/uima/TestEntityAnnotatorAE.xml", nullptr);
    aeProvider->getAE();
  });
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testEmptyMapInitialization() throws
// Exception
void OverridingParamsAEProviderTest::testEmptyMapInitialization() throw(
    runtime_error)
{
  shared_ptr<AEProvider> aeProvider = make_shared<OverridingParamsAEProvider>(
      L"/uima/TestEntityAnnotatorAE.xml", unordered_map<wstring, any>());
  shared_ptr<AnalysisEngine> analysisEngine = aeProvider->getAE();
  assertNotNull(analysisEngine);
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testOverridingParamsInitialization() throws
// Exception
void OverridingParamsAEProviderTest::testOverridingParamsInitialization() throw(
    runtime_error)
{
  unordered_map<wstring, any> runtimeParameters = unordered_map<wstring, any>();
  runtimeParameters.emplace(L"ngramsize", L"3");
  shared_ptr<AEProvider> aeProvider = make_shared<OverridingParamsAEProvider>(
      L"/uima/AggregateSentenceAE.xml", runtimeParameters);
  shared_ptr<AnalysisEngine> analysisEngine = aeProvider->getAE();
  assertNotNull(analysisEngine);
  any parameterValue = analysisEngine->getConfigParameterValue(L"ngramsize");
  assertNotNull(parameterValue);
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(static_cast<Integer>(3), stoi(parameterValue.toString()));
}
} // namespace org::apache::lucene::analysis::uima::ae