#pragma once
#include "stringhelper.h"
#include <memory>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util
{

using DocIdSet = org::apache::lucene::search::DocIdSet;

/**
 * Base test case for BitSets.
 */
template <typename T>
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Ignore public abstract class BaseBitSetTestCase<T extends
// BitSet> extends LuceneTestCase
class BaseBitSetTestCase : public LuceneTestCase
{
  static_assert(std::is_base_of<BitSet, T>::value,
                L"T must inherit from BitSet");

  /** Create a copy of the given {@link BitSet} which has <code>length</code>
   * bits. */
public:
  virtual T copyOf(std::shared_ptr<BitSet> bs, int length) = 0;

  /** Create a random set which has <code>numBitsSet</code> of its
   * <code>numBits</code> bits set. */
  static std::shared_ptr<java::util::BitSet> randomSet(int numBits,
                                                       int numBitsSet)
  {
    assert(numBitsSet <= numBits);
    std::shared_ptr<java::util::BitSet> *const set =
        std::make_shared<java::util::BitSet>(numBits);
    if (numBitsSet == numBits) {
      set->set(0, numBits);
    } else {
      for (int i = 0; i < numBitsSet; ++i) {
        while (true) {
          constexpr int o = random().nextInt(numBits);
          if (!set->get(o)) {
            set->set(o);
            break;
          }
        }
      }
    }
    return set;
  }

  /** Same as {@link #randomSet(int, int)} but given a load factor. */
  static std::shared_ptr<java::util::BitSet> randomSet(int numBits,
                                                       float percentSet)
  {
    return randomSet(numBits, static_cast<int>(percentSet * numBits));
  }

protected:
  virtual void assertEquals(std::shared_ptr<BitSet> set1, T set2, int maxDoc)
  {
    for (int i = 0; i < maxDoc; ++i) {
      assertEquals(L"Different at " + std::to_wstring(i), set1->get(i),
                   set2->get(i));
    }
  }

  /** Test the {@link BitSet#cardinality()} method. */
public:
  virtual void testCardinality() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    for (auto percentSet :
         std::deque<float>{0, 0.01f, 0.1f, 0.5f, 0.9f, 0.99f, 1.0f}) {
      std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
          randomSet(numBits, percentSet), numBits);
      T set2 = copyOf(set1, numBits);
      assertEquals(set1->cardinality(), set2->cardinality());
    }
  }

  /** Test {@link BitSet#prevSetBit(int)}. */
  virtual void testPrevSetBit() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    for (auto percentSet :
         std::deque<float>{0, 0.01f, 0.1f, 0.5f, 0.9f, 0.99f, 1.0f}) {
      std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
          randomSet(numBits, percentSet), numBits);
      T set2 = copyOf(set1, numBits);
      for (int i = 0; i < numBits; ++i) {
        // C++ TODO: There is no native C++ equivalent to 'toString':
        assertEquals(Integer::toString(i), set1->prevSetBit(i),
                     set2->prevSetBit(i));
      }
    }
  }

  /** Test {@link BitSet#nextSetBit(int)}. */
  virtual void testNextSetBit() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    for (auto percentSet :
         std::deque<float>{0, 0.01f, 0.1f, 0.5f, 0.9f, 0.99f, 1.0f}) {
      std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
          randomSet(numBits, percentSet), numBits);
      T set2 = copyOf(set1, numBits);
      for (int i = 0; i < numBits; ++i) {
        assertEquals(set1->nextSetBit(i), set2->nextSetBit(i));
      }
    }
  }

  /** Test the {@link BitSet#set} method. */
  virtual void testSet() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    std::shared_ptr<BitSet> set1 =
        std::make_shared<JavaUtilBitSet>(randomSet(numBits, 0), numBits);
    T set2 = copyOf(set1, numBits);
    constexpr int iters = 10000 + random().nextInt(10000);
    for (int i = 0; i < iters; ++i) {
      constexpr int index = random().nextInt(numBits);
      set1->set(index);
      set2->set(index);
    }
    assertEquals(set1, set2, numBits);
  }

  /** Test the {@link BitSet#clear(int)} method. */
  virtual void testClear() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    for (auto percentSet :
         std::deque<float>{0, 0.01f, 0.1f, 0.5f, 0.9f, 0.99f, 1.0f}) {
      std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
          randomSet(numBits, percentSet), numBits);
      T set2 = copyOf(set1, numBits);
      constexpr int iters = 1 + random().nextInt(numBits * 2);
      for (int i = 0; i < iters; ++i) {
        constexpr int index = random().nextInt(numBits);
        set1->clear(index);
        set2->clear(index);
      }
      assertEquals(set1, set2, numBits);
    }
  }

  /** Test the {@link BitSet#clear(int,int)} method. */
  virtual void testClearRange() 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    for (auto percentSet :
         std::deque<float>{0, 0.01f, 0.1f, 0.5f, 0.9f, 0.99f, 1.0f}) {
      std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
          randomSet(numBits, percentSet), numBits);
      T set2 = copyOf(set1, numBits);
      constexpr int iters = 1 + random().nextInt(100);
      for (int i = 0; i < iters; ++i) {
        constexpr int from = random().nextInt(numBits);
        constexpr int to = random().nextInt(numBits + 1);
        set1->clear(from, to);
        set2->clear(from, to);
        assertEquals(set1, set2, numBits);
      }
    }
  }

private:
  std::shared_ptr<DocIdSet> randomCopy(std::shared_ptr<BitSet> set,
                                       int numBits) 
  {
    switch (random().nextInt(5)) {
    case 0:
      return std::make_shared<BitDocIdSet>(set, set->cardinality());
    case 1:
      return std::make_shared<BitDocIdSet>(copyOf(set, numBits),
                                           set->cardinality());
    case 2: {
      std::shared_ptr<RoaringDocIdSet::Builder> *const builder =
          std::make_shared<RoaringDocIdSet::Builder>(numBits);
      for (int i = set->nextSetBit(0); i != DocIdSetIterator::NO_MORE_DOCS;
           i = i + 1 >= numBits ? DocIdSetIterator::NO_MORE_DOCS
                                : set->nextSetBit(i + 1)) {
        builder->add(i);
      }
      return builder->build();
    }
    case 3: {
      std::shared_ptr<FixedBitSet> fbs = std::make_shared<FixedBitSet>(numBits);
      fbs->or (std::make_shared<BitSetIterator>(set, 0));
      return std::make_shared<BitDocIdSet>(fbs);
    }
    case 4: {
      std::shared_ptr<SparseFixedBitSet> sfbs =
          std::make_shared<SparseFixedBitSet>(numBits);
      sfbs->or (std::make_shared<BitSetIterator>(set, 0));
      return std::make_shared<BitDocIdSet>(sfbs);
    }
    default:
      fail();
      return nullptr;
    }
  }

  void testOr(float load) 
  {
    constexpr int numBits = 1 + random().nextInt(100000);
    std::shared_ptr<BitSet> set1 = std::make_shared<JavaUtilBitSet>(
        randomSet(numBits, 0), numBits); // empty
    T set2 = copyOf(set1, numBits);

    constexpr int iterations = atLeast(10);
    for (int iter = 0; iter < iterations; ++iter) {
      std::shared_ptr<DocIdSet> otherSet = randomCopy(
          std::make_shared<JavaUtilBitSet>(randomSet(numBits, load), numBits),
          numBits);
      std::shared_ptr<DocIdSetIterator> otherIterator = otherSet->begin();
      if (otherIterator != nullptr) {
        set1->or (otherIterator);
        set2->or (otherSet->begin());
        assertEquals(set1, set2, numBits);
      }
    }
  }

  /** Test {@link BitSet#or(DocIdSetIterator)} on sparse sets. */
public:
  virtual void testOrSparse()  { testOr(0.001f); }

  /** Test {@link BitSet#or(DocIdSetIterator)} on dense sets. */
  virtual void testOrDense()  { testOr(0.5f); }

  /** Test {@link BitSet#or(DocIdSetIterator)} on a random density. */
  virtual void testOrRandom() 
  {
    testOr(random().nextFloat());
  }

private:
  class JavaUtilBitSet : public BitSet
  {
    GET_CLASS_NAME(JavaUtilBitSet)

  private:
    const std::shared_ptr<java::util::BitSet> bitSet;
    const int numBits;

  public:
    JavaUtilBitSet(std::shared_ptr<java::util::BitSet> bitSet, int numBits);

    void clear(int index) override;

    bool get(int index) override;

    int length() override;

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void set(int i) override;

    void clear(int startIndex, int endIndex) override;

    int cardinality() override;

    int prevSetBit(int index) override;

    int nextSetBit(int i) override;

  protected:
    std::shared_ptr<JavaUtilBitSet> shared_from_this()
    {
      return std::static_pointer_cast<JavaUtilBitSet>(
          BitSet::shared_from_this());
    }
  };

protected:
  std::shared_ptr<BaseBitSetTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseBitSetTestCase>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
