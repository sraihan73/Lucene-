#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class FixedBitSet;
}

namespace org::apache::lucene::util
{
class BitSet;
}

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

class TestFixedBitSet : public BaseBitSetTestCase<std::shared_ptr<FixedBitSet>>
{
  GET_CLASS_NAME(TestFixedBitSet)

public:
  std::shared_ptr<FixedBitSet> copyOf(std::shared_ptr<BitSet> bs,
                                      int length)  override;

  virtual void doGet(std::shared_ptr<java::util::BitSet> a,
                     std::shared_ptr<FixedBitSet> b);

  virtual void doNextSetBit(std::shared_ptr<java::util::BitSet> a,
                            std::shared_ptr<FixedBitSet> b);

  virtual void doPrevSetBit(std::shared_ptr<java::util::BitSet> a,
                            std::shared_ptr<FixedBitSet> b);

  // test interleaving different FixedBitSetIterator.next()/skipTo()
  virtual void doIterate(std::shared_ptr<java::util::BitSet> a,
                         std::shared_ptr<FixedBitSet> b,
                         int mode) ;

  virtual void doIterate1(std::shared_ptr<java::util::BitSet> a,
                          std::shared_ptr<FixedBitSet> b) ;

  virtual void doIterate2(std::shared_ptr<java::util::BitSet> a,
                          std::shared_ptr<FixedBitSet> b) ;

  virtual void doRandomSets(int maxSize, int iter, int mode) ;

  // large enough to flush obvious bugs, small enough to run in <.5 sec as part
  // of a larger testsuite.
  virtual void testSmall() ;

  // uncomment to run a bigger test (~2 minutes).
  /*
  public void testBig() {
    doRandomSets(2000,200000, 1);
    doRandomSets(2000,200000, 2);
  }
  */

  virtual void testEquals();

  virtual void testHashCodeEquals();

  virtual void testSmallBitSets();

private:
  std::shared_ptr<FixedBitSet> makeFixedBitSet(std::deque<int> &a,
                                               int numBits);

  std::shared_ptr<java::util::BitSet> makeBitSet(std::deque<int> &a);

  void checkPrevSetBitArray(std::deque<int> &a, int numBits);

public:
  void testPrevSetBit() override;

private:
  void checkNextSetBitArray(std::deque<int> &a, int numBits);

public:
  virtual void testNextBitSet();

  virtual void testEnsureCapacity();

  virtual void testBits2Words();

private:
  std::deque<int> makeIntArray(std::shared_ptr<Random> random, int count,
                                int min, int max);

  // Demonstrates that the presence of ghost bits in the last used word can
  // cause spurious failures
public:
  virtual void testIntersectionCount();

  // Demonstrates that the presence of ghost bits in the last used word can
  // cause spurious failures
  virtual void testUnionCount();

  // Demonstrates that the presence of ghost bits in the last used word can
  // cause spurious failures
  virtual void testAndNotCount();

  virtual void testCopyOf();

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<TestFixedBitSet> outerInstance;

    std::shared_ptr<org::apache::lucene::util::FixedBitSet> fixedBitSet;

  public:
    BitsAnonymousInnerClass(
        std::shared_ptr<TestFixedBitSet> outerInstance,
        std::shared_ptr<org::apache::lucene::util::FixedBitSet> fixedBitSet);

    bool get(int index) override;

    int length() override;
  };

public:
  virtual void testAsBits();

protected:
  std::shared_ptr<TestFixedBitSet> shared_from_this()
  {
    return std::static_pointer_cast<TestFixedBitSet>(
        BaseBitSetTestCase<FixedBitSet>::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
