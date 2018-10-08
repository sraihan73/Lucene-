using namespace std;

#include "TestSortedSetDocValues.h"

namespace org::apache::lucene::index
{
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestSortedSetDocValues::testNoMoreOrdsConstant()
{
  assertEquals(SortedSetDocValues::NO_MORE_ORDS, -1);
}
} // namespace org::apache::lucene::index