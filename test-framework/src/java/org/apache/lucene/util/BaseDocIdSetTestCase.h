#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <type_traits>
#include <deque>

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

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.util.BaseBitSetTestCase.randomSet;

using DocIdSet = org::apache::lucene::search::DocIdSet;

/** Base test class for {@link DocIdSet}s. */
template <typename T>
class BaseDocIdSetTestCase : public LuceneTestCase
{
  GET_CLASS_NAME(BaseDocIdSetTestCase)
  static_assert(std::is_base_of<org.apache.lucene.search.DocIdSet, T>::value,
                L"T must inherit from org.apache.lucene.search.DocIdSet");

  /** Create a copy of the given {@link BitSet} which has <code>length</code>
   * bits. */
public:
  virtual T copyOf(std::shared_ptr<BitSet> bs, int length) = 0;

  /** Test length=0. */
  virtual void testNoBit() 
  {
    std::shared_ptr<BitSet> *const bs = std::make_shared<BitSet>(1);
    constexpr T copy = copyOf(bs, 1);
    assertEquals(1, bs, copy);
  }

  /** Test length=1. */
  virtual void test1Bit() 
  {
    std::shared_ptr<BitSet> *const bs = std::make_shared<BitSet>(1);
    if (random().nextBoolean()) {
      bs->set(0);
    }
    constexpr T copy = copyOf(bs, 1);
    assertEquals(1, bs, copy);
  }

  /** Test length=2. */
  virtual void test2Bits() 
  {
    std::shared_ptr<BitSet> *const bs = std::make_shared<BitSet>(2);
    if (random().nextBoolean()) {
      bs->set(0);
    }
    if (random().nextBoolean()) {
      bs->set(1);
    }
    constexpr T copy = copyOf(bs, 2);
    assertEquals(2, bs, copy);
  }

  /** Compare the content of the set against a {@link BitSet}. */
  virtual void testAgainstBitSet() 
  {
    constexpr int numBits = TestUtil::nextInt(random(), 100, 1 << 20);
    // test various random sets with various load factors
    for (auto percentSet :
         std::deque<float>{0.0f, 0.0001f, random().nextFloat(), 0.9f, 1.0f}) {
      std::shared_ptr<BitSet> *const set = randomSet(numBits, percentSet);
      constexpr T copy = copyOf(set, numBits);
      assertEquals(numBits, set, copy);
    }
    // test one doc
    std::shared_ptr<BitSet> set = std::make_shared<BitSet>(numBits);
    set->set(0); // 0 first
    T copy = copyOf(set, numBits);
    assertEquals(numBits, set, copy);
    set->clear(0);
    set->set(random().nextInt(numBits));
    copy = copyOf(set, numBits); // then random index
    assertEquals(numBits, set, copy);
    // test regular increments
    for (int inc = 2; inc < 1000; inc += TestUtil::nextInt(random(), 1, 100)) {
      set = std::make_shared<BitSet>(numBits);
      for (int d = random().nextInt(10); d < numBits; d += inc) {
        set->set(d);
      }
      copy = copyOf(set, numBits);
      assertEquals(numBits, set, copy);
    }
  }

  /** Test ram usage estimation. */
  virtual void testRamBytesUsed() 
  {
    constexpr int iters = 100;
    for (int i = 0; i < iters; ++i) {
      constexpr int pow = random().nextInt(20);
      constexpr int maxDoc = TestUtil::nextInt(random(), 1, 1 << pow);
      constexpr int numDocs = TestUtil::nextInt(
          random(), 0,
          std::min(maxDoc, 1 << TestUtil::nextInt(random(), 0, pow)));
      std::shared_ptr<BitSet> *const set = randomSet(maxDoc, numDocs);
      std::shared_ptr<DocIdSet> *const copy = copyOf(set, maxDoc);
      constexpr int64_t actualBytes = ramBytesUsed(copy, maxDoc);
      constexpr int64_t expectedBytes = copy->ramBytesUsed();
      assertEquals(expectedBytes, actualBytes);
    }
  }

  /** Assert that the content of the {@link DocIdSet} is the same as the content
   * of the {@link BitSet}. */
  virtual void assertEquals(int numBits, std::shared_ptr<BitSet> ds1,
                            T ds2) 
  {
    // nextDoc
    std::shared_ptr<DocIdSetIterator> it2 = ds2->begin();
    if (it2 == nullptr) {
      assertEquals(-1, ds1->nextSetBit(0));
    } else {
      assertEquals(-1, it2->docID());
      for (int doc = ds1->nextSetBit(0); doc != -1;
           doc = ds1->nextSetBit(doc + 1)) {
        assertEquals(doc, it2->nextDoc());
        assertEquals(doc, it2->docID());
      }
      assertEquals(DocIdSetIterator::NO_MORE_DOCS, it2->nextDoc());
      assertEquals(DocIdSetIterator::NO_MORE_DOCS, it2->docID());
    }

    // nextDoc / advance
    it2 = ds2->begin();
    if (it2 == nullptr) {
      assertEquals(-1, ds1->nextSetBit(0));
    } else {
      for (int doc = -1; doc != DocIdSetIterator::NO_MORE_DOCS;) {
        if (random().nextBoolean()) {
          doc = ds1->nextSetBit(doc + 1);
          if (doc == -1) {
            doc = DocIdSetIterator::NO_MORE_DOCS;
          }
          assertEquals(doc, it2->nextDoc());
          assertEquals(doc, it2->docID());
        } else {
          constexpr int target =
              doc + 1 +
              random().nextInt(
                  random().nextBoolean() ? 64 : std::max(numBits / 8, 1));
          doc = ds1->nextSetBit(target);
          if (doc == -1) {
            doc = DocIdSetIterator::NO_MORE_DOCS;
          }
          assertEquals(doc, it2->advance(target));
          assertEquals(doc, it2->docID());
        }
      }
    }

    // bits()
    std::shared_ptr<Bits> *const bits = ds2->bits();
    if (bits != nullptr) {
      // test consistency between bits and iterator
      it2 = ds2->begin();
      for (int previousDoc = -1, doc = it2->nextDoc();;
           previousDoc = doc, doc = it2->nextDoc()) {
        constexpr int max =
            doc == DocIdSetIterator::NO_MORE_DOCS ? bits->length() : doc;
        for (int i = previousDoc + 1; i < max; ++i) {
          assertEquals(false, bits->get(i));
        }
        if (doc == DocIdSetIterator::NO_MORE_DOCS) {
          break;
        }
        assertEquals(true, bits->get(doc));
      }
    }
  }

private:
  class Dummy : public std::enable_shared_from_this<Dummy>
  {
    GET_CLASS_NAME(Dummy)
  public:
    // C++ TODO: Most Java annotations will not have direct C++ equivalents:
    // ORIGINAL LINE: @SuppressWarnings("unused") Object o1, o2;
    std::any o1, o2;
  };

  // same as RamUsageTester.sizeOf but tries to not take into account resources
  // that might be shared across instances
private:
  int64_t ramBytesUsed(std::shared_ptr<DocIdSet> set,
                         int length) 
  {
    std::shared_ptr<Dummy> dummy = std::make_shared<Dummy>();
    dummy->o1 = copyOf(std::make_shared<BitSet>(length), length);
    dummy->o2 = set;
    int64_t bytes1 = RamUsageTester::sizeOf(dummy);
    dummy->o2 = nullptr;
    int64_t bytes2 = RamUsageTester::sizeOf(dummy);
    return bytes1 - bytes2;
  }

protected:
  std::shared_ptr<BaseDocIdSetTestCase> shared_from_this()
  {
    return std::static_pointer_cast<BaseDocIdSetTestCase>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
