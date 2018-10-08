using namespace std;

#include "TestDocIdSetBuilder.h"

namespace org::apache::lucene::util
{
using PointValues = org::apache::lucene::index::PointValues;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

void TestDocIdSetBuilder::testEmpty() 
{
  assertEquals(
      nullptr,
      (make_shared<DocIdSetBuilder>(1 + random()->nextInt(1000)))->build());
}

void TestDocIdSetBuilder::assertEquals(
    shared_ptr<DocIdSet> d1, shared_ptr<DocIdSet> d2) 
{
  if (d1 == nullptr) {
    if (d2 != nullptr) {
      assertEquals(DocIdSetIterator::NO_MORE_DOCS, d2->begin().nextDoc());
    }
  } else if (d2 == nullptr) {
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, d1->begin().nextDoc());
  } else {
    shared_ptr<DocIdSetIterator> i1 = d1->begin();
    shared_ptr<DocIdSetIterator> i2 = d2->begin();
    for (int doc = i1->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = i1->nextDoc()) {
      assertEquals(doc, i2->nextDoc());
    }
    assertEquals(DocIdSetIterator::NO_MORE_DOCS, i2->nextDoc());
  }
}

void TestDocIdSetBuilder::testSparse() 
{
  constexpr int maxDoc = 1000000 + random()->nextInt(1000000);
  shared_ptr<DocIdSetBuilder> builder = make_shared<DocIdSetBuilder>(maxDoc);
  constexpr int numIterators = 1 + random()->nextInt(10);
  shared_ptr<FixedBitSet> *const ref = make_shared<FixedBitSet>(maxDoc);
  for (int i = 0; i < numIterators; ++i) {
    constexpr int baseInc = 200000 + random()->nextInt(10000);
    shared_ptr<RoaringDocIdSet::Builder> b =
        make_shared<RoaringDocIdSet::Builder>(maxDoc);
    for (int doc = random()->nextInt(100); doc < maxDoc;
         doc += baseInc + random()->nextInt(10000)) {
      b->add(doc);
      ref->set(doc);
    }
    builder->add(b->build()->begin());
  }
  shared_ptr<DocIdSet> result = builder->build();
  assertTrue(std::dynamic_pointer_cast<IntArrayDocIdSet>(result) != nullptr);
  assertEquals(make_shared<BitDocIdSet>(ref), result);
}

void TestDocIdSetBuilder::testDense() 
{
  constexpr int maxDoc = 1000000 + random()->nextInt(1000000);
  shared_ptr<DocIdSetBuilder> builder = make_shared<DocIdSetBuilder>(maxDoc);
  constexpr int numIterators = 1 + random()->nextInt(10);
  shared_ptr<FixedBitSet> *const ref = make_shared<FixedBitSet>(maxDoc);
  for (int i = 0; i < numIterators; ++i) {
    shared_ptr<RoaringDocIdSet::Builder> b =
        make_shared<RoaringDocIdSet::Builder>(maxDoc);
    for (int doc = random()->nextInt(1000); doc < maxDoc;
         doc += 1 + random()->nextInt(100)) {
      b->add(doc);
      ref->set(doc);
    }
    builder->add(b->build()->begin());
  }
  shared_ptr<DocIdSet> result = builder->build();
  assertTrue(std::dynamic_pointer_cast<BitDocIdSet>(result) != nullptr);
  assertEquals(make_shared<BitDocIdSet>(ref), result);
}

void TestDocIdSetBuilder::testRandom() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 1, 10000000);
  for (int i = 1; i < maxDoc / 2; i <<= 1) {
    constexpr int numDocs = TestUtil::nextInt(random(), 1, i);
    shared_ptr<FixedBitSet> *const docs = make_shared<FixedBitSet>(maxDoc);
    int c = 0;
    while (c < numDocs) {
      constexpr int d = random()->nextInt(maxDoc);
      if (docs->get(d) == false) {
        docs->set(d);
        c += 1;
      }
    }

    const std::deque<int> array_ =
        std::deque<int>(numDocs + random()->nextInt(100));
    shared_ptr<DocIdSetIterator> it = make_shared<BitSetIterator>(docs, 0LL);
    int j = 0;
    for (int doc = it->nextDoc(); doc != DocIdSetIterator::NO_MORE_DOCS;
         doc = it->nextDoc()) {
      array_[j++] = doc;
    }
    assertEquals(numDocs, j);

    // add some duplicates
    while (j < array_.size()) {
      array_[j++] = array_[random()->nextInt(numDocs)];
    }

    // shuffle
    for (j = array_.size() - 1; j >= 1; --j) {
      constexpr int k = random()->nextInt(j);
      int tmp = array_[j];
      array_[j] = array_[k];
      array_[k] = tmp;
    }

    // add docs out of order
    shared_ptr<DocIdSetBuilder> builder = make_shared<DocIdSetBuilder>(maxDoc);
    for (j = 0; j < array_.size();) {
      constexpr int l = TestUtil::nextInt(random(), 1, array_.size() - j);
      shared_ptr<DocIdSetBuilder::BulkAdder> adder = nullptr;
      for (int k = 0, budget = 0; k < l; ++k) {
        if (budget == 0 || rarely()) {
          budget = TestUtil::nextInt(random(), 1, l - k + 5);
          adder = builder->grow(budget);
        }
        adder->add(array_[j++]);
        budget--;
      }
    }

    shared_ptr<DocIdSet> *const expected = make_shared<BitDocIdSet>(docs);
    shared_ptr<DocIdSet> *const actual = builder->build();
    assertEquals(expected, actual);
  }
}

void TestDocIdSetBuilder::testMisleadingDISICost() 
{
  constexpr int maxDoc = TestUtil::nextInt(random(), 1000, 10000);
  shared_ptr<DocIdSetBuilder> builder = make_shared<DocIdSetBuilder>(maxDoc);
  shared_ptr<FixedBitSet> expected = make_shared<FixedBitSet>(maxDoc);

  for (int i = 0; i < 10; ++i) {
    shared_ptr<FixedBitSet> *const docs = make_shared<FixedBitSet>(maxDoc);
    constexpr int numDocs = random()->nextInt(maxDoc / 1000);
    for (int j = 0; j < numDocs; ++j) {
      docs->set(random()->nextInt(maxDoc));
    }
    expected->or (docs);
    // We provide a cost of 0 here to make sure the builder can deal with wrong
    // costs
    builder->add(make_shared<BitSetIterator>(docs, 0LL));
  }

  assertEquals(make_shared<BitDocIdSet>(expected), builder->build());
}

void TestDocIdSetBuilder::testEmptyPoints() 
{
  shared_ptr<PointValues> values = make_shared<DummyPointValues>(0, 0);
  shared_ptr<DocIdSetBuilder> builder =
      make_shared<DocIdSetBuilder>(1, values, L"foo");
  assertEquals(1, builder->numValuesPerDoc, 0);
}

void TestDocIdSetBuilder::testLeverageStats() 
{
  // single-valued points
  shared_ptr<PointValues> values = make_shared<DummyPointValues>(42, 42);
  shared_ptr<DocIdSetBuilder> builder =
      make_shared<DocIdSetBuilder>(100, values, L"foo");
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertFalse(builder->multivalued);
  shared_ptr<DocIdSetBuilder::BulkAdder> adder = builder->grow(2);
  adder->add(5);
  adder->add(7);
  shared_ptr<DocIdSet> set = builder->build();
  assertTrue(std::dynamic_pointer_cast<BitDocIdSet>(set) != nullptr);
  assertEquals(2, set->begin().cost());

  // multi-valued points
  values = make_shared<DummyPointValues>(42, 63);
  builder = make_shared<DocIdSetBuilder>(100, values, L"foo");
  assertEquals(1.5, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);
  adder = builder->grow(2);
  adder->add(5);
  adder->add(7);
  set = builder->build();
  assertTrue(std::dynamic_pointer_cast<BitDocIdSet>(set) != nullptr);
  assertEquals(1,
               set->begin().cost()); // it thinks the same doc was added twice

  // incomplete stats
  values = make_shared<DummyPointValues>(42, -1);
  builder = make_shared<DocIdSetBuilder>(100, values, L"foo");
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);

  values = make_shared<DummyPointValues>(-1, 84);
  builder = make_shared<DocIdSetBuilder>(100, values, L"foo");
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);

  // single-valued terms
  shared_ptr<Terms> terms = make_shared<DummyTerms>(42, 42);
  builder = make_shared<DocIdSetBuilder>(100, terms);
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertFalse(builder->multivalued);
  adder = builder->grow(2);
  adder->add(5);
  adder->add(7);
  set = builder->build();
  assertTrue(std::dynamic_pointer_cast<BitDocIdSet>(set) != nullptr);
  assertEquals(2, set->begin().cost());

  // multi-valued terms
  terms = make_shared<DummyTerms>(42, 63);
  builder = make_shared<DocIdSetBuilder>(100, terms);
  assertEquals(1.5, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);
  adder = builder->grow(2);
  adder->add(5);
  adder->add(7);
  set = builder->build();
  assertTrue(std::dynamic_pointer_cast<BitDocIdSet>(set) != nullptr);
  assertEquals(1,
               set->begin().cost()); // it thinks the same doc was added twice

  // incomplete stats
  terms = make_shared<DummyTerms>(42, -1);
  builder = make_shared<DocIdSetBuilder>(100, terms);
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);

  terms = make_shared<DummyTerms>(-1, 84);
  builder = make_shared<DocIdSetBuilder>(100, terms);
  assertEquals(1, builder->numValuesPerDoc, 0);
  assertTrue(builder->multivalued);
}

TestDocIdSetBuilder::DummyTerms::DummyTerms(int docCount, int64_t numValues)
    : docCount(docCount), numValues(numValues)
{
}

shared_ptr<TermsEnum>
TestDocIdSetBuilder::DummyTerms::iterator() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestDocIdSetBuilder::DummyTerms::size() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t
TestDocIdSetBuilder::DummyTerms::getSumTotalTermFreq() 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestDocIdSetBuilder::DummyTerms::getSumDocFreq() 
{
  return numValues;
}

int TestDocIdSetBuilder::DummyTerms::getDocCount() 
{
  return docCount;
}

bool TestDocIdSetBuilder::DummyTerms::hasFreqs()
{
  throw make_shared<UnsupportedOperationException>();
}

bool TestDocIdSetBuilder::DummyTerms::hasOffsets()
{
  throw make_shared<UnsupportedOperationException>();
}

bool TestDocIdSetBuilder::DummyTerms::hasPositions()
{
  throw make_shared<UnsupportedOperationException>();
}

bool TestDocIdSetBuilder::DummyTerms::hasPayloads()
{
  throw make_shared<UnsupportedOperationException>();
}

TestDocIdSetBuilder::DummyPointValues::DummyPointValues(int docCount,
                                                        int64_t numPoints)
    : docCount(docCount), numPoints(numPoints)
{
}

void TestDocIdSetBuilder::DummyPointValues::intersect(
    shared_ptr<PointValues::IntersectVisitor> visitor) 
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestDocIdSetBuilder::DummyPointValues::estimatePointCount(
    shared_ptr<PointValues::IntersectVisitor> visitor)
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
TestDocIdSetBuilder::DummyPointValues::getMinPackedValue() 
{
  throw make_shared<UnsupportedOperationException>();
}

std::deque<char>
TestDocIdSetBuilder::DummyPointValues::getMaxPackedValue() 
{
  throw make_shared<UnsupportedOperationException>();
}

int TestDocIdSetBuilder::DummyPointValues::getNumDimensions() 
{
  throw make_shared<UnsupportedOperationException>();
}

int TestDocIdSetBuilder::DummyPointValues::getBytesPerDimension() throw(
    IOException)
{
  throw make_shared<UnsupportedOperationException>();
}

int64_t TestDocIdSetBuilder::DummyPointValues::size() { return numPoints; }

int TestDocIdSetBuilder::DummyPointValues::getDocCount() { return docCount; }
} // namespace org::apache::lucene::util