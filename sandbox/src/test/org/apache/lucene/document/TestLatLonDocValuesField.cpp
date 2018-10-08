using namespace std;

#include "TestLatLonDocValuesField.h"

namespace org::apache::lucene::document
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLatLonDocValuesField::testToString() 
{
  // looks crazy due to lossiness
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"LatLonDocValuesField <field:18.313693958334625,-65.22744401358068>",
      (make_shared<LatLonDocValuesField>(L"field", 18.313694, -65.227444))
          ->toString());

  // sort field
  // C++ TODO: There is no native C++ equivalent to 'toString':
  assertEquals(
      L"<distance:\"field\" latitude=18.0 longitude=19.0>",
      LatLonDocValuesField::newDistanceSort(L"field", 18.0, 19.0)->toString());
}
} // namespace org::apache::lucene::document