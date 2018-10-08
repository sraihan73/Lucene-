using namespace std;

#include "TestJaroWinklerDistance.h"

namespace org::apache::lucene::search::spell
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestJaroWinklerDistance::testGetDistance()
{
  float d = sd->getDistance(L"al", L"al");
  assertTrue(d == 1.0f);
  d = sd->getDistance(L"martha", L"marhta");
  assertTrue(d > 0.961 && d < 0.962);
  d = sd->getDistance(L"jones", L"johnson");
  assertTrue(d > 0.832 && d < 0.833);
  d = sd->getDistance(L"abcvwxyz", L"cabvwxyz");
  assertTrue(d > 0.958 && d < 0.959);
  d = sd->getDistance(L"dwayne", L"duane");
  assertTrue(d > 0.84 && d < 0.841);
  d = sd->getDistance(L"dixon", L"dicksonx");
  assertTrue(d > 0.813 && d < 0.814);
  d = sd->getDistance(L"fvie", L"ten");
  assertTrue(d == 0.0f);
  float d1 = sd->getDistance(L"zac ephron", L"zac efron");
  float d2 = sd->getDistance(L"zac ephron", L"kai ephron");
  assertTrue(d1 > d2);
  d1 = sd->getDistance(L"brittney spears", L"britney spears");
  d2 = sd->getDistance(L"brittney spears", L"brittney startzman");
  assertTrue(d1 > d2);
}
} // namespace org::apache::lucene::search::spell