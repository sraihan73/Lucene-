using namespace std;

#include "TestBlockJoinSelector.h"

namespace org::apache::lucene::search::join
{
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;
using DocValues = org::apache::lucene::index::DocValues;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using BitSet = org::apache::lucene::util::BitSet;
using Bits = org::apache::lucene::util::Bits;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

void TestBlockJoinSelector::testDocsWithValue()
{
  shared_ptr<BitSet> *const parents = make_shared<FixedBitSet>(20);
  parents->set(0);
  parents->set(5);
  parents->set(6);
  parents->set(10);
  parents->set(15);
  parents->set(19);

  shared_ptr<BitSet> *const children = make_shared<FixedBitSet>(20);
  children->set(2);
  children->set(3);
  children->set(4);
  children->set(12);
  children->set(17);

  shared_ptr<BitSet> *const childDocsWithValue = make_shared<FixedBitSet>(20);
  childDocsWithValue->set(2);
  childDocsWithValue->set(3);
  childDocsWithValue->set(4);
  childDocsWithValue->set(8);
  childDocsWithValue->set(16);

  shared_ptr<Bits> *const docsWithValue =
      BlockJoinSelector::wrap(childDocsWithValue, parents, children);
  assertFalse(docsWithValue->get(0));
  assertTrue(docsWithValue->get(5));
  assertFalse(docsWithValue->get(6));
  assertFalse(docsWithValue->get(10));
  assertFalse(docsWithValue->get(15));
  assertFalse(docsWithValue->get(19));
}

void TestBlockJoinSelector::assertNoMoreDoc(shared_ptr<DocIdSetIterator> sdv,
                                            int maxDoc) 
{
  shared_ptr<Random> r = random();
  if (r->nextBoolean()) {
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, sdv->nextDoc());
  } else {
    if (r->nextBoolean()) {
      assertEquals(DocIdSetIterator::NO_MORE_DOCS,
                   sdv->advance(sdv->docID() +
                                random()->nextInt(maxDoc - sdv->docID())));
    } else {
      constexpr int noMatchDoc =
          sdv->docID() + random()->nextInt(maxDoc - sdv->docID() - 1) + 1;
      assertFalse(advanceExact(sdv, noMatchDoc));
      assertEquals(noMatchDoc, sdv->docID());
      if (r->nextBoolean()) {
        assertEquals(DocIdSetIterator::NO_MORE_DOCS, sdv->nextDoc());
      }
    }
  }
}

int TestBlockJoinSelector::nextDoc(shared_ptr<DocIdSetIterator> sdv,
                                   int docId) 
{
  shared_ptr<Random> r = random();
  if (r->nextBoolean()) {
    return sdv->nextDoc();
  } else {
    if (r->nextBoolean()) {
      return sdv->advance(sdv->docID() +
                          random()->nextInt(docId - sdv->docID() - 1) + 1);
    } else {
      if (r->nextBoolean()) {
        constexpr int noMatchDoc =
            sdv->docID() + random()->nextInt(docId - sdv->docID() - 1) + 1;
        assertFalse(advanceExact(sdv, noMatchDoc));
        assertEquals(noMatchDoc, sdv->docID());
      }
      assertTrue(advanceExact(sdv, docId));
      return sdv->docID();
    }
  }
}

bool TestBlockJoinSelector::advanceExact(shared_ptr<DocIdSetIterator> sdv,
                                         int target) 
{
  return std::dynamic_pointer_cast<SortedDocValues>(sdv) != nullptr
             ? (std::static_pointer_cast<SortedDocValues>(sdv))
                   ->advanceExact(target)
             : (std::static_pointer_cast<NumericDocValues>(sdv))
                   ->advanceExact(target);
}

void TestBlockJoinSelector::testSortedSelector() 
{
  shared_ptr<BitSet> *const parents = make_shared<FixedBitSet>(20);
  parents->set(0);
  parents->set(5);
  parents->set(6);
  parents->set(10);
  parents->set(15);
  parents->set(19);

  shared_ptr<BitSet> *const children = make_shared<FixedBitSet>(20);
  children->set(2);
  children->set(3);
  children->set(4);
  children->set(12);
  children->set(17);

  const std::deque<int> ords = std::deque<int>(20);
  Arrays::fill(ords, -1);
  ords[2] = 5;
  ords[3] = 7;
  ords[4] = 3;
  ords[12] = 10;
  ords[18] = 10;

  shared_ptr<SortedDocValues> *const mins = BlockJoinSelector::wrap(
      DocValues::singleton(make_shared<CannedSortedDocValues>(ords)),
      BlockJoinSelector::Type::MIN, parents, children);
  assertEquals(5, nextDoc(mins, 5));
  assertEquals(3, mins->ordValue());
  assertEquals(15, nextDoc(mins, 15));
  assertEquals(10, mins->ordValue());
  assertNoMoreDoc(mins, 20);

  shared_ptr<SortedDocValues> *const maxs = BlockJoinSelector::wrap(
      DocValues::singleton(make_shared<CannedSortedDocValues>(ords)),
      BlockJoinSelector::Type::MAX, parents, children);
  assertEquals(5, nextDoc(maxs, 5));
  assertEquals(7, maxs->ordValue());
  assertEquals(15, nextDoc(maxs, 15));
  assertEquals(10, maxs->ordValue());
  assertNoMoreDoc(maxs, 20);
}

TestBlockJoinSelector::CannedSortedDocValues::CannedSortedDocValues(
    std::deque<int> &ords)
    : ords(ords)
{
}

int TestBlockJoinSelector::CannedSortedDocValues::docID() { return docID_; }

int TestBlockJoinSelector::CannedSortedDocValues::nextDoc()
{
  while (true) {
    docID_++;
    if (docID_ == ords.size()) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    if (ords[docID_] != -1) {
      break;
    }
  }
  return docID_;
}

int TestBlockJoinSelector::CannedSortedDocValues::advance(int target)
{
  if (target >= ords.size()) {
    docID_ = NO_MORE_DOCS;
  } else {
    docID_ = target;
    if (ords[docID_] == -1) {
      nextDoc();
    }
  }
  return docID_;
}

bool TestBlockJoinSelector::CannedSortedDocValues::advanceExact(
    int target) 
{
  docID_ = target;
  return ords[docID_] != -1;
}

int TestBlockJoinSelector::CannedSortedDocValues::ordValue()
{
  assert(ords[docID_] != -1);
  return ords[docID_];
}

int64_t TestBlockJoinSelector::CannedSortedDocValues::cost() { return 5; }

shared_ptr<BytesRef>
TestBlockJoinSelector::CannedSortedDocValues::lookupOrd(int ord)
{
  throw make_shared<UnsupportedOperationException>();
}

int TestBlockJoinSelector::CannedSortedDocValues::getValueCount() { return 11; }

void TestBlockJoinSelector::testNumericSelector() 
{
  shared_ptr<BitSet> *const parents = make_shared<FixedBitSet>(20);
  parents->set(0);
  parents->set(5);
  parents->set(6);
  parents->set(10);
  parents->set(15);
  parents->set(19);

  shared_ptr<BitSet> *const children = make_shared<FixedBitSet>(20);
  children->set(2);
  children->set(3);
  children->set(4);
  children->set(12);
  children->set(17);

  const std::deque<int64_t> longs = std::deque<int64_t>(20);
  shared_ptr<BitSet> *const docsWithValue = make_shared<FixedBitSet>(20);
  docsWithValue->set(2);
  longs[2] = 5;
  docsWithValue->set(3);
  longs[3] = 7;
  docsWithValue->set(4);
  longs[4] = 3;
  docsWithValue->set(12);
  longs[12] = 10;
  docsWithValue->set(18);
  longs[18] = 10;

  shared_ptr<NumericDocValues> *const mins = BlockJoinSelector::wrap(
      DocValues::singleton(
          make_shared<CannedNumericDocValues>(longs, docsWithValue)),
      BlockJoinSelector::Type::MIN, parents, children);
  assertEquals(5, nextDoc(mins, 5));
  assertEquals(3, mins->longValue());
  assertEquals(15, nextDoc(mins, 15));
  assertEquals(10, mins->longValue());
  assertNoMoreDoc(mins, 20);

  shared_ptr<NumericDocValues> *const maxs = BlockJoinSelector::wrap(
      DocValues::singleton(
          make_shared<CannedNumericDocValues>(longs, docsWithValue)),
      BlockJoinSelector::Type::MAX, parents, children);
  assertEquals(5, nextDoc(maxs, 5));
  assertEquals(7, maxs->longValue());
  assertEquals(15, nextDoc(maxs, 15));
  assertEquals(10, maxs->longValue());
  assertNoMoreDoc(maxs, 20);
}

TestBlockJoinSelector::CannedNumericDocValues::CannedNumericDocValues(
    std::deque<int64_t> &values, shared_ptr<Bits> docsWithValue)
    : docsWithValue(docsWithValue), values(values)
{
}

int TestBlockJoinSelector::CannedNumericDocValues::docID() { return docID_; }

int TestBlockJoinSelector::CannedNumericDocValues::nextDoc()
{
  while (true) {
    docID_++;
    if (docID_ == values.size()) {
      docID_ = NO_MORE_DOCS;
      break;
    }
    if (docsWithValue->get(docID_)) {
      break;
    }
  }
  return docID_;
}

int TestBlockJoinSelector::CannedNumericDocValues::advance(int target)
{
  if (target >= values.size()) {
    docID_ = NO_MORE_DOCS;
    return docID_;
  } else {
    docID_ = target - 1;
    return nextDoc();
  }
}

bool TestBlockJoinSelector::CannedNumericDocValues::advanceExact(
    int target) 
{
  docID_ = target;
  return docsWithValue->get(docID_);
}

int64_t TestBlockJoinSelector::CannedNumericDocValues::longValue()
{
  return values[docID_];
}

int64_t TestBlockJoinSelector::CannedNumericDocValues::cost() { return 5; }
} // namespace org::apache::lucene::search::join