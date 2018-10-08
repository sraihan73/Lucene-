using namespace std;

#include "TestLongBitSet.h"

namespace org::apache::lucene::util
{

void TestLongBitSet::doGet(shared_ptr<java::util::BitSet> a,
                           shared_ptr<LongBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int64_t max = b->length();
  for (int i = 0; i < max; i++) {
    if (a->get(i) != b->get(i)) {
      fail(L"mismatch: BitSet=[" + to_wstring(i) + L"]=" + a->get(i));
    }
  }
}

void TestLongBitSet::doNextSetBit(shared_ptr<java::util::BitSet> a,
                                  shared_ptr<LongBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = -1;
  int64_t bb = -1;
  do {
    aa = a->nextSetBit(aa + 1);
    bb = bb < b->length() - 1 ? b->nextSetBit(bb + 1) : -1;
    assertEquals(aa, bb);
  } while (aa >= 0);
}

void TestLongBitSet::doPrevSetBit(shared_ptr<java::util::BitSet> a,
                                  shared_ptr<LongBitSet> b)
{
  assertEquals(a->cardinality(), b->cardinality());
  int aa = a->size() + random()->nextInt(100);
  int64_t bb = aa;
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

void TestLongBitSet::doRandomSets(int maxSize, int iter,
                                  int mode) 
{
  shared_ptr<java::util::BitSet> a0 = nullptr;
  shared_ptr<LongBitSet> b0 = nullptr;

  for (int i = 0; i < iter; i++) {
    int sz = TestUtil::nextInt(random(), 2, maxSize);
    shared_ptr<java::util::BitSet> a = make_shared<java::util::BitSet>(sz);
    shared_ptr<LongBitSet> b = make_shared<LongBitSet>(sz);

    // test the various ways of setting bits
    if (sz > 0) {
      int nOper = random()->nextInt(sz);
      for (int j = 0; j < nOper; j++) {
        int idx;

        idx = random()->nextInt(sz);
        a->set(idx);
        b->set(idx);

        idx = random()->nextInt(sz);
        a->clear(idx);
        b->clear(idx);

        idx = random()->nextInt(sz);
        a->flip(idx, idx + 1);
        b->flip(idx, idx + 1);

        idx = random()->nextInt(sz);
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
    fromIndex = random()->nextInt(sz / 2);
    toIndex = fromIndex + random()->nextInt(sz - fromIndex);
    shared_ptr<java::util::BitSet> aa =
        std::static_pointer_cast<java::util::BitSet>(a->clone());
    aa->flip(fromIndex, toIndex);
    shared_ptr<LongBitSet> bb = b->clone();
    bb->flip(fromIndex, toIndex);

    fromIndex = random()->nextInt(sz / 2);
    toIndex = fromIndex + random()->nextInt(sz - fromIndex);
    aa = std::static_pointer_cast<java::util::BitSet>(a->clone());
    aa->clear(fromIndex, toIndex);
    bb = b->clone();
    bb->clear(fromIndex, toIndex);

    doNextSetBit(aa, bb); // a problem here is from clear() or nextSetBit

    doPrevSetBit(aa, bb);

    fromIndex = random()->nextInt(sz / 2);
    toIndex = fromIndex + random()->nextInt(sz - fromIndex);
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

      shared_ptr<LongBitSet> b_and = b->clone();
      assertEquals(b, b_and);
      b_and->and (b0);
      shared_ptr<LongBitSet> b_or = b->clone();
      b_or->or (b0);
      shared_ptr<LongBitSet> b_xor = b->clone();
      b_xor->xor (b0);
      shared_ptr<LongBitSet> b_andn = b->clone();
      b_andn->andNot(b0);

      assertEquals(a0->cardinality(), b0->cardinality());
      assertEquals(a_or->cardinality(), b_or->cardinality());

      assertEquals(a_and->cardinality(), b_and->cardinality());
      assertEquals(a_or->cardinality(), b_or->cardinality());
      assertEquals(a_xor->cardinality(), b_xor->cardinality());
      assertEquals(a_andn->cardinality(), b_andn->cardinality());
    }

    a0 = a;
    b0 = b;
  }
}

void TestLongBitSet::testSmall() 
{
  constexpr int iters = TEST_NIGHTLY ? atLeast(1000) : 100;
  doRandomSets(atLeast(1200), iters, 1);
  doRandomSets(atLeast(1200), iters, 2);
}

void TestLongBitSet::testEquals()
{
  // This test can't handle numBits==0:
  constexpr int numBits = random()->nextInt(2000) + 1;
  shared_ptr<LongBitSet> b1 = make_shared<LongBitSet>(numBits);
  shared_ptr<LongBitSet> b2 = make_shared<LongBitSet>(numBits);
  assertTrue(b1->equals(b2));
  assertTrue(b2->equals(b1));
  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {
    int idx = random()->nextInt(numBits);
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

void TestLongBitSet::testHashCodeEquals()
{
  // This test can't handle numBits==0:
  constexpr int numBits = random()->nextInt(2000) + 1;
  shared_ptr<LongBitSet> b1 = make_shared<LongBitSet>(numBits);
  shared_ptr<LongBitSet> b2 = make_shared<LongBitSet>(numBits);
  assertTrue(b1->equals(b2));
  assertTrue(b2->equals(b1));
  for (int iter = 0; iter < 10 * RANDOM_MULTIPLIER; iter++) {
    int idx = random()->nextInt(numBits);
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

void TestLongBitSet::testTooLarge()
{
  runtime_error e = expectThrows(invalid_argument::typeid, [&]() {
    make_shared<LongBitSet>(LongBitSet::MAX_NUM_BITS + 1);
  });
  assertTrue(e.what()->startsWith(L"numBits must be 0 .. "));
}

void TestLongBitSet::testNegativeNumBits()
{
  runtime_error e = expectThrows(invalid_argument::typeid,
                                 [&]() { make_shared<LongBitSet>(-17); });
  assertTrue(e.what()->startsWith(L"numBits must be 0 .. "));
}

void TestLongBitSet::testSmallBitSets()
{
  // Make sure size 0-10 bit sets are OK:
  for (int numBits = 0; numBits < 10; numBits++) {
    shared_ptr<LongBitSet> b1 = make_shared<LongBitSet>(numBits);
    shared_ptr<LongBitSet> b2 = make_shared<LongBitSet>(numBits);
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

shared_ptr<LongBitSet> TestLongBitSet::makeLongBitSet(std::deque<int> &a,
                                                      int numBits)
{
  shared_ptr<LongBitSet> bs;
  if (random()->nextBoolean()) {
    int bits2words = LongBitSet::bits2words(numBits);
    std::deque<int64_t> words(bits2words + random()->nextInt(100));
    bs = make_shared<LongBitSet>(words, numBits);
  } else {
    bs = make_shared<LongBitSet>(numBits);
  }
  for (auto e : a) {
    bs->set(e);
  }
  return bs;
}

shared_ptr<java::util::BitSet> TestLongBitSet::makeBitSet(std::deque<int> &a)
{
  shared_ptr<java::util::BitSet> bs = make_shared<java::util::BitSet>();
  for (auto e : a) {
    bs->set(e);
  }
  return bs;
}

void TestLongBitSet::checkPrevSetBitArray(std::deque<int> &a, int numBits)
{
  shared_ptr<LongBitSet> obs = makeLongBitSet(a, numBits);
  shared_ptr<java::util::BitSet> bs = makeBitSet(a);
  doPrevSetBit(bs, obs);
}

void TestLongBitSet::testPrevSetBit()
{
  checkPrevSetBitArray(std::deque<int>(), 0);
  checkPrevSetBitArray(std::deque<int>{0}, 1);
  checkPrevSetBitArray(std::deque<int>{0, 2}, 3);
}

void TestLongBitSet::checkNextSetBitArray(std::deque<int> &a, int numBits)
{
  shared_ptr<LongBitSet> obs = makeLongBitSet(a, numBits);
  shared_ptr<java::util::BitSet> bs = makeBitSet(a);
  doNextSetBit(bs, obs);
}

void TestLongBitSet::testNextBitSet()
{
  std::deque<int> setBits(0 + random()->nextInt(1000));
  for (int i = 0; i < setBits.size(); i++) {
    setBits[i] = random()->nextInt(setBits.size());
  }
  checkNextSetBitArray(setBits, setBits.size() + random()->nextInt(10));

  checkNextSetBitArray(std::deque<int>(0),
                       setBits.size() + random()->nextInt(10));
}

void TestLongBitSet::testEnsureCapacity()
{
  shared_ptr<LongBitSet> bits = make_shared<LongBitSet>(5);
  bits->set(1);
  bits->set(4);

  shared_ptr<LongBitSet> newBits =
      LongBitSet::ensureCapacity(bits, 8); // grow within the word
  assertTrue(newBits->get(1));
  assertTrue(newBits->get(4));
  newBits->clear(1);
  // we align to 64-bits, so even though it shouldn't have, it re-allocated a
  // long[1]
  assertTrue(bits->get(1));
  assertFalse(newBits->get(1));

  newBits->set(1);
  newBits = LongBitSet::ensureCapacity(newBits, newBits->length() - 2); // reuse
  assertTrue(newBits->get(1));

  bits->set(1);
  newBits = LongBitSet::ensureCapacity(bits, 72); // grow beyond one word
  assertTrue(newBits->get(1));
  assertTrue(newBits->get(4));
  newBits->clear(1);
  // we grew the long[], so it's not shared
  assertTrue(bits->get(1));
  assertFalse(newBits->get(1));
}

// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Nightly public void testHugeCapacity()
void TestLongBitSet::testHugeCapacity()
{
  int64_t moreThanMaxInt =
      static_cast<int64_t>(numeric_limits<int>::max()) + 5;

  shared_ptr<LongBitSet> bits = make_shared<LongBitSet>(42);

  assertEquals(42, bits->length());

  shared_ptr<LongBitSet> hugeBits =
      LongBitSet::ensureCapacity(bits, moreThanMaxInt);

  assertTrue(hugeBits->length() >= moreThanMaxInt);
}

void TestLongBitSet::testBits2Words()
{
  assertEquals(0, LongBitSet::bits2words(0));
  assertEquals(1, LongBitSet::bits2words(1));
  // ...
  assertEquals(1, LongBitSet::bits2words(64));
  assertEquals(2, LongBitSet::bits2words(65));
  // ...
  assertEquals(2, LongBitSet::bits2words(128));
  assertEquals(3, LongBitSet::bits2words(129));
  // ...
  assertEquals(1 << (31 - 6),
               LongBitSet::bits2words(
                   static_cast<int64_t>(numeric_limits<int>::max()) +
                   1)); // == 1L << 31
  assertEquals((1 << (31 - 6)) + 1,
               LongBitSet::bits2words(
                   static_cast<int64_t>(numeric_limits<int>::max()) +
                   2)); // == (1L << 31) + 1
  // ...
  assertEquals(1 << (32 - 6), LongBitSet::bits2words(1LL << 32));
  assertEquals((1 << (32 - 6)) + 1, LongBitSet::bits2words((1LL << 32)) + 1);

  // ensure the claimed max num_bits doesn't throw exc; we can't enforce exact
  // values here because the value variees with JVM:
  assertTrue(LongBitSet::bits2words(LongBitSet::MAX_NUM_BITS) > 0);
}
} // namespace org::apache::lucene::util