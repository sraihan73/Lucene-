using namespace std;

#include "TestLatLonPoint.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLatLonPoint::testToString() 
{
  // looks crazy due to lossiness
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"LatLonPoint <field:18.313693958334625,-65.22744401358068>",
      (make_shared<LatLonPoint>(L"field", 18.313694, -65.227444))->toString());

  // looks crazy due to lossiness
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"field:[18.000000016763806 TO 18.999999999068677],[-65.9999999217689 TO "
      L"-65.00000006519258]",
      LatLonPoint::newBoxQuery(L"field", 18, 19, -66, -65)->toString());

  // distance query does not quantize inputs
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(L"field:18.0,19.0 +/- 25.0 meters",
               LatLonPoint::newDistanceQuery(L"field", 18, 19, 25)->toString());
}
} // namespace org::apache::lucene::document