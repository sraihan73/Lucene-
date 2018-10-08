using namespace std;

#include "TestDuelingCodecsAtNight.h"

namespace org::apache::lucene::index
{
using org::apache::lucene::util::LuceneTestCase::Nightly;
using org::apache::lucene::util::LuceneTestCase::SuppressCodecs;

void TestDuelingCodecsAtNight::testBigEquals() 
{
  int numdocs = atLeast(2000);
  createRandomIndex(numdocs, leftWriter, seed);
  createRandomIndex(numdocs, rightWriter, seed);

  leftReader = leftWriter->getReader();
  rightReader = rightWriter->getReader();

  assertReaderEquals(info, leftReader, rightReader);
}
} // namespace org::apache::lucene::index