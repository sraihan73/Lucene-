using namespace std;

#include "TestFixedBitSet.h"

namespace org::apache::lucene::util
{
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;

shared_ptr<FixedBitSet> TestFixedBitSet::copyOf(shared_ptr<BitSet> bs,
                                                int length) 
{
  shared_ptr<FixedBitSet> *const set = make_shared<FixedBitSet>(length);
  for (int doc = bs->nextSetBit(0); doc != DocIdSetIterator::NO_MORE_DOCS;
       doc = doc + 1 >= length ? DocIdSetIterator::NO_MORE_DOCS
                               : bs->nextSetBit(doc + 1)) {
    set->set(doc);
  }
  return set;
}

void TestFixedBitSet::doGet(shared_ptr<java::util::BitSet> a,
                            shared_ptr<FixedBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int max = b->length();
  for (int i = 0; i < max; i++) {
    if (a->get(i) != b->get(i)) {
      fail(L"mismatch: BitSet=[" + to_wstring(i) + L"]=" + a->get(i));
    }
  }
}

void TestFixedBitSet::doNextSetBit(shared_ptr<java::util::BitSet> a,
                                   shared_ptr<FixedBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = -1;
  int bb = -1;
  do {
    aa = a->nextSetBit(aa + 1);
    if (aa == -1) {
      aa = DocIdSetIterator::NO_MORE_DOCS;
    }
    bb = bb < b->length() - 1 ? b->nextSetBit(bb + 1)
                              : DocIdSetIterator::NO_MORE_DOCS;
    assertEquals(aa, bb);
  } while (aa != DocIdSetIterator::NO_MORE_DOCS);
}

void TestFixedBitSet::doPrevSetBit(shared_ptr<java::util::BitSet> a,
                                   shared_ptr<FixedBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = a->size() + random().nextInt(100);
  int bb = aa;
  do {
    // aa = a.prevSetBit(aa-1);
    aa--;
    while ((aa >= 0) && (!a->get(aa))) {
      aa--;
    }
    if (b->length() == 0) {
      bb = -1;
    } else if (bb > b->length() - 1) {
      bb = b->prevSetBit(b->length() - 1);
    } else if (bb < 1) {
      bb = -1;
    } else {
      bb = bb >= 1 ? b->prevSetBit(bb - 1) : -1;
    }
    assertEquals(aa, bb);
  } while (aa >= 0);
}

void TestFixedBitSet::doIterate(shared_ptr<java::util::BitSet> a,
                                shared_ptr<FixedBitSet> b,
                                int mode) 
{
  if (mode == 1) {
    doIterate1(a, b);
  }
  if (mode == 2) {
    doIterate2(a, b);
  }
}

void TestFixedBitSet::doIterate1(shared_ptr<java::util::BitSet> a,
                                 shared_ptr<FixedBitSet> b) 
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = -1, bb = -1;
  shared_ptr<DocIdSetIterator> iterator = make_shared<BitSetIterator>(b, 0);
  do {
    aa = a->nextSetBit(aa + 1);
    bb = (bb < b->length() && random().nextBoolean())
             ? iterator->nextDoc()
             : iterator->advance(bb + 1);
    assertEquals(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
  } while (aa >= 0);
}

void TestFixedBitSet::doIterate2(shared_ptr<java::util::BitSet> a,
                                 shared_ptr<FixedBitSet> b) 
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = -1, bb = -1;
  shared_ptr<DocIdSetIterator> iterator = make_shared<BitSetIterator>(b, 0);
  do {
    aa = a->nextSetBit(aa + 1);
    bb = random().nextBoolean() ? iterator->nextDoc()
                                : iterator->advance(bb + 1);
    assertEquals(aa == -1 ? DocIdSetIterator::NO_MORE_DOCS : aa, bb);
  } while (aa >= 0);
}

void TestFixedBitSet::doRandomSets(int maxSize, int iter,
                                   int mode) 
{
  shared_ptr<java::util::BitSet> a0 = nullptr;
  shared_ptr<FixedBitSet> b0 = nullptr;

  for (int i = 0; i < iter; i++) {
    int sz = TestUtil::nextInt(random(), 2, maxSize);
    shared_ptr<java::util::BitSet> a = make_shared<java::util::BitSet>(sz);
    shared_ptr<FixedBitSet> b = make_shared<FixedBitSet>(sz);

    // test the various ways of setting bits
    if (sz > 0) {
      int nOper = random().nextInt(sz);
      for (int j = 0; j < nOper; j++) {
        int idx;

        idx = random().nextInt(sz);
        a->set(idx);
        b->set(idx);

        idx = random().nextInt(sz);
        a->clear(idx);
        b->clear(idx);

        idx = random().nextInt(sz);
        a->flip(idx, idx + 1);
        b->flip(idx, idx + 1);

        idx = random().nextInt(sz);
        a->flip(idx);
        b->flip(idx);

        bool val2 = b->get(idx);
        bool val = b->getAndSet(idx);
        assertTrue(val2 == val);
        assertTrue(b->get(idx));

        if (!val) {
          b->clear(idx);
        }
        assertTrue(b->get(idx) == val);
      }
    }

    // test that the various ways of accessing the bits are equivalent
    doGet(a, b);

    // test ranges, including possible extension
    int fromIndex, toIndex;
    fromIndex = random().nextInt(sz / 2);
    toIndex = fromIndex + random().nextInt(sz - fromIndex);
    shared_ptr<java::util::BitSet> aa =
        std::static_pointer_cast<java::util::BitSet>(a->clone());
    aa->flip(fromIndex, toIndex);
    shared_ptr<FixedBitSet> bb = b->clone();
    bb->flip(fromIndex, toIndex);

    doIterate(aa, bb, mode); // a problem here is from flip or doIterate

    fromIndex = random().nextInt(sz / 2);
    toIndex = fromIndex + random().nextInt(sz - fromIndex);
    aa = std::static_pointer_cast<java::util::BitSet>(a->clone());
    aa->clear(fromIndex, toIndex);
    bb = b->clone();
    bb->clear(fromIndex, toIndex);

    doNextSetBit(aa, bb); // a problem here is from clear() or nextSetBit

    doPrevSetBit(aa, bb);

    fromIndex = random().nextInt(sz / 2);
    toIndex = fromIndex + random().nextInt(sz - fromIndex);
    aa = std::static_pointer_cast<java::util::BitSet>(a->clone());
    aa->set(fromIndex, toIndex);
    bb = b->clone();
    bb->set(fromIndex, toIndex);

    doNextSetBit(aa, bb); // a problem here is from set() or nextSetBit

    doPrevSetBit(aa, bb);

    if (b0 != nullptr && b0->length() <= b->length()) {
      assertEquals(a->cardinality(), b->cardinality());

      shared_ptr<java::util::BitSet> a_and =
          std::static_pointer_cast<java::util::BitSet>(a->clone());
      a_and->and (a0);
      shared_ptr<java::util::BitSet> a_or =
          std::static_pointer_cast<java::util::BitSet>(a->clone());
      a_or->or (a0);
      shared_ptr<java::util::BitSet> a_xor =
          std::static_pointer_cast<java::util::BitSet>(a->clone());
      a_xor->xor (a0);
      shared_ptr<java::util::BitSet> a_andn =
          std::static_pointer_cast<java::util::BitSet>(a->clone());
      a_andn->andNot(a0);

      shared_ptr<FixedBitSet> b_and = b->clone();
      assertEquals(b, b_and);
      b_and->and (b0);
      shared_ptr<FixedBitSet> b_or = b->clone();
      b_or->or (b0);
      shared_ptr<FixedBitSet> b_xor = b->clone();
      b_xor->xor (b0);
      shared_ptr<FixedBitSet> b_andn = b->clone();
      b_andn->andNot(b0);

      assertEquals(a0->cardinality(), b0->cardinality());
      assertEquals(a_or->cardinality(), b_or->cardinality());

      doIterate(a_and, b_and, mode);
      doIterate(a_or, b_or, mode);
      doIterate(a_andn, b_andn, mode);
      doIterate(a_xor, b_xor, mode);

      assertEquals(a_and->cardinality(), b_and->cardinality());
      assertEquals(a_or->cardinality(), b_or->cardinality());
      assertEquals(a_xor->cardinality(), b_xor->cardinality());
      assertEquals(a_andn->cardinality(), b_andn->cardinality());
    }

    a0 = a;
    b0 = b;
  }
}

void TestFixedBitSet::testSmall() 
{
  constexpr int iters = TEST_NIGHTLY ? atLeast(1000) : 100;
  doRandomSets(atLeast(1200), iters, 1);
  doRandomSets(atLeast(1200), iters, 2);
}

void TestFixedBitSet::testEquals()
{
  // This test can't handle numBits==0:
  constexpr int numBits = random().nextInt(2000) + 1;
  shared_ptr<FixedBitSet> b1 = make_shared<FixedBitSet>(numBits);
  shared_ptr<FixedBitSet> b2 = make_shared<FixedBitSet>(numBits);
  assertTrue(b1->equals(b2));
  assertTrue(b2->equals(b1));
  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {
    int idx = random().nextInt(numBits);
    if (!b1->get(idx)) {
      b1->set(idx);
      assertFalse(b1->equals(b2));
      assertFalse(b2->equals(b1));
      b2->set(idx);
      assertTrue(b1->equals(b2));
      assertTrue(b2->equals(b1));
    }
  }

  // try different type of object
  assertFalse(b1->equals(any()));
}

void TestFixedBitSet::testHashCodeEquals()
{
  // This test can't handle numBits==0:
  constexpr int numBits = random().nextInt(2000) + 1;
  shared_ptr<FixedBitSet> b1 = make_shared<FixedBitSet>(numBits);
  shared_ptr<FixedBitSet> b2 = make_shared<FixedBitSet>(numBits);
  assertTrue(b1->equals(b2));
  assertTrue(b2->equals(b1));
  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {
    int idx = random().nextInt(numBits);
    if (!b1->get(idx)) {
      b1->set(idx);
      assertFalse(b1->equals(b2));
      assertFalse(b1->hashCode() == b2->hashCode());
      b2->set(idx);
      assertEquals(b1, b2);
      assertEquals(b1->hashCode(), b2->hashCode());
    }
  }
}

void TestFixedBitSet::testSmallBitSets()
{
  // Make sure size 0-10 bit sets are OK:
  for (int numBits = 0; numBits < 10; numBits++) {
    shared_ptr<FixedBitSet> b1 = make_shared<FixedBitSet>(numBits);
    shared_ptr<FixedBitSet> b2 = make_shared<FixedBitSet>(numBits);
    assertTrue(b1->equals(b2));
    assertEquals(b1->hashCode(), b2->hashCode());
    assertEquals(0, b1->cardinality());
    if (numBits > 0) {
      b1->set(0, numBits);
      assertEquals(numBits, b1->cardinality());
      b1->flip(0, numBits);
      assertEquals(0, b1->cardinality());
    }
  }
}

shared_ptr<FixedBitSet> TestFixedBitSet::makeFixedBitSet(std::deque<int> &a,
                                                         int numBits)
{
  shared_ptr<FixedBitSet> bs;
  if (random().nextBoolean()) {
    int bits2words = FixedBitSet::bits2words(numBits);
    std::deque<int64_t> words(bits2words + random().nextInt(100));
    bs = make_shared<FixedBitSet>(words, numBits);
  } else {
    bs = make_shared<FixedBitSet>(numBits);
  }
  for (auto e : a) {
    bs->set(e);
  }
  return bs;
}

shared_ptr<java::util::BitSet> TestFixedBitSet::makeBitSet(std::deque<int> &a)
{
  shared_ptr<java::util::BitSet> bs = make_shared<java::util::BitSet>();
  for (auto e : a) {
    bs->set(e);
  }
  return bs;
}

void TestFixedBitSet::checkPrevSetBitArray(std::deque<int> &a, int numBits)
{
  shared_ptr<FixedBitSet> obs = makeFixedBitSet(a, numBits);
  shared_ptr<java::util::BitSet> bs = makeBitSet(a);
  doPrevSetBit(bs, obs);
}

void TestFixedBitSet::testPrevSetBit()
{
  checkPrevSetBitArray(std::deque<int>(), 0);
  checkPrevSetBitArray(std::deque<int>{0}, 1);
  checkPrevSetBitArray(std::deque<int>{0, 2}, 3);
}

void TestFixedBitSet::checkNextSetBitArray(std::deque<int> &a, int numBits)
{
  shared_ptr<FixedBitSet> obs = makeFixedBitSet(a, numBits);
  shared_ptr<java::util::BitSet> bs = makeBitSet(a);
  doNextSetBit(bs, obs);
}

void TestFixedBitSet::testNextBitSet()
{
  std::deque<int> setBits(0 + random().nextInt(1000));
  for (int i = 0; i < setBits.size(); i++) {
    setBits[i] = random().nextInt(setBits.size());
  }
  checkNextSetBitArray(setBits, setBits.size() + random().nextInt(10));

  checkNextSetBitArray(std::deque<int>(0),
                       setBits.size() + random().nextInt(10));
}

void TestFixedBitSet::testEnsureCapacity()
{
  shared_ptr<FixedBitSet> bits = make_shared<FixedBitSet>(5);
  bits->set(1);
  bits->set(4);

  shared_ptr<FixedBitSet> newBits =
      FixedBitSet::ensureCapacity(bits, 8); // grow within the word
  assertTrue(newBits->get(1));
  assertTrue(newBits->get(4));
  newBits->clear(1);
  // we align to 64-bits, so even though it shouldn't have, it re-allocated a
  // long[1]
  assertTrue(bits->get(1));
  assertFalse(newBits->get(1));

  newBits->set(1);
  newBits =
      FixedBitSet::ensureCapacity(newBits, newBits->length() - 2); // reuse
  assertTrue(newBits->get(1));

  bits->set(1);
  newBits = FixedBitSet::ensureCapacity(bits, 72); // grow beyond one word
  assertTrue(newBits->get(1));
  assertTrue(newBits->get(4));
  newBits->clear(1);
  // we grew the long[], so it's not shared
  assertTrue(bits->get(1));
  assertFalse(newBits->get(1));
}

void TestFixedBitSet::testBits2Words()
{
  assertEquals(0, FixedBitSet::bits2words(0));
  assertEquals(1, FixedBitSet::bits2words(1));
  // ...
  assertEquals(1, FixedBitSet::bits2words(64));
  assertEquals(2, FixedBitSet::bits2words(65));
  // ...
  assertEquals(2, FixedBitSet::bits2words(128));
  assertEquals(3, FixedBitSet::bits2words(129));
  // ...
  assertEquals(1024, FixedBitSet::bits2words(65536));
  assertEquals(1025, FixedBitSet::bits2words(65537));
  // ...
  assertEquals(1 << (31 - 6),
               FixedBitSet::bits2words(numeric_limits<int>::max()));
}

std::deque<int> TestFixedBitSet::makeIntArray(shared_ptr<Random> random,
                                               int count, int min, int max)
{
  std::deque<int> rv(count);

  for (int i = 0; i < count; ++i) {
    rv[i] = TestUtil::nextInt(random, min, max);
  }

  return rv;
}

void TestFixedBitSet::testIntersectionCount()
{
  shared_ptr<Random> random = random();

  int numBits1 = TestUtil::nextInt(random, 1000, 2000);
  int numBits2 = TestUtil::nextInt(random, 1000, 2000);

  int count1 = TestUtil::nextInt(random, 0, numBits1 - 1);
  int count2 = TestUtil::nextInt(random, 0, numBits2 - 1);

  std::deque<int> bits1 = makeIntArray(random, count1, 0, numBits1 - 1);
  std::deque<int> bits2 = makeIntArray(random, count2, 0, numBits2 - 1);

  shared_ptr<FixedBitSet> fixedBitSet1 = makeFixedBitSet(bits1, numBits1);
  shared_ptr<FixedBitSet> fixedBitSet2 = makeFixedBitSet(bits2, numBits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertTrue(fixedBitSet1.cardinality() <= bits1.length);
  // assertTrue(fixedBitSet2.cardinality() <= bits2.length);

  int64_t intersectionCount =
      FixedBitSet::intersectionCount(fixedBitSet1, fixedBitSet2);

  shared_ptr<java::util::BitSet> bitSet1 = makeBitSet(bits1);
  shared_ptr<java::util::BitSet> bitSet2 = makeBitSet(bits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertEquals(bitSet1.cardinality(), fixedBitSet1.cardinality());
  // assertEquals(bitSet2.cardinality(), fixedBitSet2.cardinality());

  bitSet1->and (bitSet2);

  assertEquals(bitSet1->cardinality(), intersectionCount);
}

void TestFixedBitSet::testUnionCount()
{
  shared_ptr<Random> random = random();

  int numBits1 = TestUtil::nextInt(random, 1000, 2000);
  int numBits2 = TestUtil::nextInt(random, 1000, 2000);

  int count1 = TestUtil::nextInt(random, 0, numBits1 - 1);
  int count2 = TestUtil::nextInt(random, 0, numBits2 - 1);

  std::deque<int> bits1 = makeIntArray(random, count1, 0, numBits1 - 1);
  std::deque<int> bits2 = makeIntArray(random, count2, 0, numBits2 - 1);

  shared_ptr<FixedBitSet> fixedBitSet1 = makeFixedBitSet(bits1, numBits1);
  shared_ptr<FixedBitSet> fixedBitSet2 = makeFixedBitSet(bits2, numBits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertTrue(fixedBitSet1.cardinality() <= bits1.length);
  // assertTrue(fixedBitSet2.cardinality() <= bits2.length);

  int64_t unionCount = FixedBitSet::unionCount(fixedBitSet1, fixedBitSet2);

  shared_ptr<java::util::BitSet> bitSet1 = makeBitSet(bits1);
  shared_ptr<java::util::BitSet> bitSet2 = makeBitSet(bits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertEquals(bitSet1.cardinality(), fixedBitSet1.cardinality());
  // assertEquals(bitSet2.cardinality(), fixedBitSet2.cardinality());

  bitSet1->or (bitSet2);

  assertEquals(bitSet1->cardinality(), unionCount);
}

void TestFixedBitSet::testAndNotCount()
{
  shared_ptr<Random> random = random();

  int numBits1 = TestUtil::nextInt(random, 1000, 2000);
  int numBits2 = TestUtil::nextInt(random, 1000, 2000);

  int count1 = TestUtil::nextInt(random, 0, numBits1 - 1);
  int count2 = TestUtil::nextInt(random, 0, numBits2 - 1);

  std::deque<int> bits1 = makeIntArray(random, count1, 0, numBits1 - 1);
  std::deque<int> bits2 = makeIntArray(random, count2, 0, numBits2 - 1);

  shared_ptr<FixedBitSet> fixedBitSet1 = makeFixedBitSet(bits1, numBits1);
  shared_ptr<FixedBitSet> fixedBitSet2 = makeFixedBitSet(bits2, numBits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertTrue(fixedBitSet1.cardinality() <= bits1.length);
  // assertTrue(fixedBitSet2.cardinality() <= bits2.length);

  int64_t andNotCount = FixedBitSet::andNotCount(fixedBitSet1, fixedBitSet2);

  shared_ptr<java::util::BitSet> bitSet1 = makeBitSet(bits1);
  shared_ptr<java::util::BitSet> bitSet2 = makeBitSet(bits2);

  // If ghost bits are present, these may fail too, but that's not what we want
  // to demonstrate here
  // assertEquals(bitSet1.cardinality(), fixedBitSet1.cardinality());
  // assertEquals(bitSet2.cardinality(), fixedBitSet2.cardinality());

  bitSet1->andNot(bitSet2);

  assertEquals(bitSet1->cardinality(), andNotCount);
}

void TestFixedBitSet::testCopyOf()
{
  shared_ptr<Random> random = random();
  int numBits = TestUtil::nextInt(random, 1000, 2000);
  int count = TestUtil::nextInt(random, 0, numBits - 1);
  std::deque<int> bits = makeIntArray(random, count, 0, numBits - 1);
  shared_ptr<FixedBitSet> fixedBitSet = make_shared<FixedBitSet>(numBits);
  for (auto e : bits) {
    fixedBitSet->set(e);
  }
  for (auto readOnly : std::deque<bool>{false, true}) {
    shared_ptr<Bits> bitsToCopy =
        readOnly ? fixedBitSet->asReadOnlyBits() : fixedBitSet;
    shared_ptr<FixedBitSet> mutableCopy = FixedBitSet::copyOf(bitsToCopy);
    assertNotSame(mutableCopy, bitsToCopy);
    assertEquals(mutableCopy, fixedBitSet);
  }

  shared_ptr<Bits> *const bitsToCopy =
      make_shared<BitsAnonymousInnerClass>(shared_from_this(), fixedBitSet);
  shared_ptr<FixedBitSet> mutableCopy = FixedBitSet::copyOf(bitsToCopy);

  assertNotSame(bitsToCopy, mutableCopy);
  assertNotSame(fixedBitSet, mutableCopy);
  assertEquals(mutableCopy, fixedBitSet);
}

TestFixedBitSet::BitsAnonymousInnerClass::BitsAnonymousInnerClass(
    shared_ptr<TestFixedBitSet> outerInstance,
    shared_ptr<org::apache::lucene::util::FixedBitSet> fixedBitSet)
{
  this->outerInstance = outerInstance;
  this->fixedBitSet = fixedBitSet;
}

bool TestFixedBitSet::BitsAnonymousInnerClass::get(int index)
{
  return fixedBitSet->get(index);
}

int TestFixedBitSet::BitsAnonymousInnerClass::length()
{
  return fixedBitSet->length();
}

void TestFixedBitSet::testAsBits()
{
  shared_ptr<FixedBitSet> set = make_shared<FixedBitSet>(10);
  set->set(3);
  set->set(4);
  set->set(9);
  shared_ptr<Bits> bits = set->asReadOnlyBits();
  assertFalse(std::dynamic_pointer_cast<FixedBitSet>(bits) != nullptr);
  assertEquals(set->length(), bits->length());
  for (int i = 0; i < set->length(); ++i) {
    assertEquals(set->get(i), bits->get(i));
  }
  // Further changes are reflected
  set->set(5);
  assertTrue(bits->get(5));
}
} // namespace org::apache::lucene::util