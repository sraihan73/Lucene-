using namespace std;

#include "TestLevenshteinDistance.h"

namespace org::apache::lucene::search::spell
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestLevenshteinDistance::testGetDistance()
{
  float d = sd->getDistance(L"al", L"al");
  assertEquals(d, 1.0f, 0.001);
  d = sd->getDistance(L"martha", L"marhta");
  assertEquals(d, 0.6666, 0.001);
  d = sd->getDistance(L"jones", L"johnson");
  assertEquals(d, 0.4285, 0.001);
  d = sd->getDistance(L"abcvwxyz", L"cabvwxyz");
  assertEquals(d, 0.75, 0.001);
  d = sd->getDistance(L"dwayne", L"duane");
  assertEquals(d, 0.666, 0.001);
  d = sd->getDistance(L"dixon", L"dicksonx");
  assertEquals(d, 0.5, 0.001);
  d = sd->getDistance(L"six", L"ten");
  assertEquals(d, 0, 0.001);
  float d1 = sd->getDistance(L"zac ephron", L"zac efron");
  float d2 = sd->getDistance(L"zac ephron", L"kai ephron");
  assertEquals(d1, d2, 0.001);
  d1 = sd->getDistance(L"brittney spears", L"britney spears");
  d2 = sd->getDistance(L"brittney spears", L"brittney startzman");
  assertTrue(d1 > d2);
}

void TestLevenshteinDistance::testEmpty() 
{
  float d = sd->getDistance(L"", L"al");
  assertEquals(d, 0.0f, 0.001);
}
} // namespace org::apache::lucene::search::spell