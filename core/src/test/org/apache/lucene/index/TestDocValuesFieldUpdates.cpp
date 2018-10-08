using namespace std;

#include "TestDocValuesFieldUpdates.h"

namespace org::apache::lucene::index
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestDocValuesFieldUpdates::testMergeIterator()
{
  shared_ptr<NumericDocValuesFieldUpdates> updates1 =
      make_shared<NumericDocValuesFieldUpdates>(0, L"test", 6);
  shared_ptr<NumericDocValuesFieldUpdates> updates2 =
      make_shared<NumericDocValuesFieldUpdates>(1, L"test", 6);
  shared_ptr<NumericDocValuesFieldUpdates> updates3 =
      make_shared<NumericDocValuesFieldUpdates>(2, L"test", 6);
  shared_ptr<NumericDocValuesFieldUpdates> updates4 =
      make_shared<NumericDocValuesFieldUpdates>(2, L"test", 6);

  updates1->add(0, 1);
  updates1->add(4, 0);
  updates1->add(1, 4);
  updates1->add(2, 5);
  updates1->add(4, 9);
  assertTrue(updates1->any());

  updates2->add(0, 18);
  updates2->add(1, 7);
  updates2->add(2, 19);
  updates2->add(5, 24);
  assertTrue(updates2->any());

  updates3->add(2, 42);
  assertTrue(updates3->any());
  assertFalse(updates4->any());
  updates1->finish();
  updates2->finish();
  updates3->finish();
  updates4->finish();
  deque<std::shared_ptr<DocValuesFieldUpdates::Iterator>> iterators =
      Arrays::asList(updates1->begin(), updates2->begin(), updates3->begin(),
                     updates4->begin());
  Collections::shuffle(iterators, random());
  shared_ptr<DocValuesFieldUpdates::Iterator> iterator =
      DocValuesFieldUpdates::mergedIterator(iterators.toArray(
          std::deque<std::shared_ptr<DocValuesFieldUpdates::Iterator>>(0)));
  assertEquals(0, iterator->nextDoc());
  assertEquals(18, iterator->longValue());
  assertEquals(1, iterator->nextDoc());
  assertEquals(7, iterator->longValue());
  assertEquals(2, iterator->nextDoc());
  assertEquals(42, iterator->longValue());
  assertEquals(4, iterator->nextDoc());
  assertEquals(9, iterator->longValue());
  assertEquals(5, iterator->nextDoc());
  assertEquals(24, iterator->longValue());
  assertEquals(DocIdSetIterator::NO_MORE_DOCS, iterator->nextDoc());
}
} // namespace org::apache::lucene::index