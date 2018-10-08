#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/SparseFixedBitSet.h"

#include  "core/src/java/org/apache/lucene/util/BitSet.h"

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

class TestSparseFixedBitSet
    : public BaseBitSetTestCase<std::shared_ptr<SparseFixedBitSet>>
{
  GET_CLASS_NAME(TestSparseFixedBitSet)

public:
  std::shared_ptr<SparseFixedBitSet>
  copyOf(std::shared_ptr<BitSet> bs, int length)  override;

protected:
  void assertEquals(std::shared_ptr<BitSet> set1,
                    std::shared_ptr<SparseFixedBitSet> set2,
                    int maxDoc) override;

public:
  virtual void testApproximateCardinality();

  virtual void testApproximateCardinalityOnDenseSet();

protected:
  std::shared_ptr<TestSparseFixedBitSet> shared_from_this()
  {
    return std::static_pointer_cast<TestSparseFixedBitSet>(
        BaseBitSetTestCase<SparseFixedBitSet>::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/
