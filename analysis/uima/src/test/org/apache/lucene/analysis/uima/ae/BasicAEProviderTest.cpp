using namespace std;

#include "BasicAEProviderTest.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/AEProvider.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/BasicAEProvider.h"

namespace org::apache::lucene::analysis::uima::ae
{
using org::apache::uima::analysis_engine::AnalysisEngine;
using org::junit::Test;
//    import static org.junit.Assert.assertNotNull;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testBasicInitialization() throws Exception
void BasicAEProviderTest::testBasicInitialization() 
{
  shared_ptr<AEProvider> basicAEProvider =
      make_shared<BasicAEProvider>(L"/uima/TestEntityAnnotatorAE.xml");
  shared_ptr<AnalysisEngine> analysisEngine = basicAEProvider->getAE();
  assertNotNull(analysisEngine);
}
} // namespace org::apache::lucene::analysis::uima::ae