using namespace std;

#include "TestNGramDistance.h"

namespace org::apache::lucene::search::spell
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestNGramDistance::testGetDistance1()
{
  shared_ptr<StringDistance> nsd = make_shared<NGramDistance>(1);
  float d = nsd->getDistance(L"al", L"al");
  assertEquals(d, 1.0f, 0.001);
  d = nsd->getDistance(L"a", L"a");
  assertEquals(d, 1.0f, 0.001);
  d = nsd->getDistance(L"b", L"a");
  assertEquals(d, 0.0f, 0.001);
  d = nsd->getDistance(L"martha", L"marhta");
  assertEquals(d, 0.6666, 0.001);
  d = nsd->getDistance(L"jones", L"johnson");
  assertEquals(d, 0.4285, 0.001);
  d = nsd->getDistance(L"natural", L"contrary");
  assertEquals(d, 0.25, 0.001);
  d = nsd->getDistance(L"abcvwxyz", L"cabvwxyz");
  assertEquals(d, 0.75, 0.001);
  d = nsd->getDistance(L"dwayne", L"duane");
  assertEquals(d, 0.666, 0.001);
  d = nsd->getDistance(L"dixon", L"dicksonx");
  assertEquals(d, 0.5, 0.001);
  d = nsd->getDistance(L"six", L"ten");
  assertEquals(d, 0, 0.001);
  float d1 = nsd->getDistance(L"zac ephron", L"zac efron");
  float d2 = nsd->getDistance(L"zac ephron", L"kai ephron");
  assertEquals(d1, d2, 0.001);
  d1 = nsd->getDistance(L"brittney spears", L"britney spears");
  d2 = nsd->getDistance(L"brittney spears", L"brittney startzman");
  assertTrue(d1 > d2);
  d1 = nsd->getDistance(L"12345678", L"12890678");
  d2 = nsd->getDistance(L"12345678", L"72385698");
  assertEquals(d1, d2, 001);
}

void TestNGramDistance::testGetDistance2()
{
  shared_ptr<StringDistance> sd = make_shared<NGramDistance>(2);
  float d = sd->getDistance(L"al", L"al");
  assertEquals(d, 1.0f, 0.001);
  d = sd->getDistance(L"a", L"a");
  assertEquals(d, 1.0f, 0.001);
  d = sd->getDistance(L"b", L"a");
  assertEquals(d, 0.0f, 0.001);
  d = sd->getDistance(L"a", L"aa");
  assertEquals(d, 0.5f, 0.001);
  d = sd->getDistance(L"martha", L"marhta");
  assertEquals(d, 0.6666, 0.001);
  d = sd->getDistance(L"jones", L"johnson");
  assertEquals(d, 0.4285, 0.001);
  d = sd->getDistance(L"natural", L"contrary");
  assertEquals(d, 0.25, 0.001);
  d = sd->getDistance(L"abcvwxyz", L"cabvwxyz");
  assertEquals(d, 0.625, 0.001);
  d = sd->getDistance(L"dwayne", L"duane");
  assertEquals(d, 0.5833, 0.001);
  d = sd->getDistance(L"dixon", L"dicksonx");
  assertEquals(d, 0.5, 0.001);
  d = sd->getDistance(L"six", L"ten");
  assertEquals(d, 0, 0.001);
  float d1 = sd->getDistance(L"zac ephron", L"zac efron");
  float d2 = sd->getDistance(L"zac ephron", L"kai ephron");
  assertTrue(d1 > d2);
  d1 = sd->getDistance(L"brittney spears", L"britney spears");
  d2 = sd->getDistance(L"brittney spears", L"brittney startzman");
  assertTrue(d1 > d2);
  d1 = sd->getDistance(L"0012345678", L"0012890678");
  d2 = sd->getDistance(L"0012345678", L"0072385698");
  assertEquals(d1, d2, 0.001);
}

void TestNGramDistance::testGetDistance3()
{
  shared_ptr<StringDistance> sd = make_shared<NGramDistance>(3);
  float d = sd->getDistance(L"al", L"al");
  assertEquals(d, 1.0f, 0.001);
  d = sd->getDistance(L"a", L"a");
  assertEquals(d, 1.0f, 0.001);
  d = sd->getDistance(L"b", L"a");
  assertEquals(d, 0.0f, 0.001);
  d = sd->getDistance(L"martha", L"marhta");
  assertEquals(d, 0.7222, 0.001);
  d = sd->getDistance(L"jones", L"johnson");
  assertEquals(d, 0.4762, 0.001);
  d = sd->getDistance(L"natural", L"contrary");
  assertEquals(d, 0.2083, 0.001);
  d = sd->getDistance(L"abcvwxyz", L"cabvwxyz");
  assertEquals(d, 0.5625, 0.001);
  d = sd->getDistance(L"dwayne", L"duane");
  assertEquals(d, 0.5277, 0.001);
  d = sd->getDistance(L"dixon", L"dicksonx");
  assertEquals(d, 0.4583, 0.001);
  d = sd->getDistance(L"six", L"ten");
  assertEquals(d, 0, 0.001);
  float d1 = sd->getDistance(L"zac ephron", L"zac efron");
  float d2 = sd->getDistance(L"zac ephron", L"kai ephron");
  assertTrue(d1 > d2);
  d1 = sd->getDistance(L"brittney spears", L"britney spears");
  d2 = sd->getDistance(L"brittney spears", L"brittney startzman");
  assertTrue(d1 > d2);
  d1 = sd->getDistance(L"0012345678", L"0012890678");
  d2 = sd->getDistance(L"0012345678", L"0072385698");
  assertTrue(d1 < d2);
}

void TestNGramDistance::testEmpty() 
{
  shared_ptr<StringDistance> nsd = make_shared<NGramDistance>(1);
  float d = nsd->getDistance(L"", L"al");
  assertEquals(d, 0.0f, 0.001);
}
} // namespace org::apache::lucene::search::spell