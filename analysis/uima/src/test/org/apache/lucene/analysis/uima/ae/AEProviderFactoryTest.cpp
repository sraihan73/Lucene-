using namespace std;

#include "AEProviderFactoryTest.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/AEProvider.h"
#include "../../../../../../../java/org/apache/lucene/analysis/uima/ae/AEProviderFactory.h"

namespace org::apache::lucene::analysis::uima::ae
{
using org::junit::Test;
//    import static org.junit.Assert.assertTrue;

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCorrectCaching() throws Exception
void AEProviderFactoryTest::testCorrectCaching() 
{
  shared_ptr<AEProvider> aeProvider =
      AEProviderFactory::getInstance()->getAEProvider(
          L"/uima/TestAggregateSentenceAE.xml");
  assertTrue(aeProvider == AEProviderFactory::getInstance()->getAEProvider(
                               L"/uima/TestAggregateSentenceAE.xml"));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Test public void testCorrectCachingWithParameters() throws
// Exception
void AEProviderFactoryTest::testCorrectCachingWithParameters() throw(
    runtime_error)
{
  shared_ptr<AEProvider> aeProvider =
      AEProviderFactory::getInstance()->getAEProvider(
          L"prefix", L"/uima/TestAggregateSentenceAE.xml",
          unordered_map<wstring, any>());
  assertTrue(aeProvider == AEProviderFactory::getInstance()->getAEProvider(
                               L"prefix", L"/uima/TestAggregateSentenceAE.xml",
                               unordered_map<wstring, any>()));
}
} // namespace org::apache::lucene::analysis::uima::ae