#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/LongBitSet.h"

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

class TestLongBitSet : public LuceneTestCase
{
  GET_CLASS_NAME(TestLongBitSet)

public:
  virtual void doGet(std::shared_ptr<java::util::BitSet> a,
                     std::shared_ptr<LongBitSet> b);

  virtual void doNextSetBit(std::shared_ptr<java::util::BitSet> a,
                            std::shared_ptr<LongBitSet> b);

  virtual void doPrevSetBit(std::shared_ptr<java::util::BitSet> a,
                            std::shared_ptr<LongBitSet> b);

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

  virtual void testTooLarge();

  virtual void testNegativeNumBits();

  virtual void testSmallBitSets();

private:
  std::shared_ptr<LongBitSet> makeLongBitSet(std::deque<int> &a, int numBits);

  std::shared_ptr<java::util::BitSet> makeBitSet(std::deque<int> &a);

  void checkPrevSetBitArray(std::deque<int> &a, int numBits);

public:
  virtual void testPrevSetBit();

private:
  void checkNextSetBitArray(std::deque<int> &a, int numBits);

public:
  virtual void testNextBitSet();

  virtual void testEnsureCapacity();

  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @Nightly public void testHugeCapacity()
  virtual void testHugeCapacity();

  virtual void testBits2Words();

protected:
  std::shared_ptr<TestLongBitSet> shared_from_this()
  {
    return std::static_pointer_cast<TestLongBitSet>(
        LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
