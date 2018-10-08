#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/bloom/FuzzySet.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"

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
namespace org::apache::lucene::codecs::bloom
{
using FieldInfo = org::apache::lucene::index::FieldInfo;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;

/**
 * Class used to create index-time {@link FuzzySet} appropriately configured for
 * each field. Also called to right-size bitsets for serialization.
 * @lucene.experimental
 */
class BloomFilterFactory
    : public std::enable_shared_from_this<BloomFilterFactory>
{
  GET_CLASS_NAME(BloomFilterFactory)

  /**
   *
   * @param state  The content to be indexed
   * @param info
   *          the field requiring a BloomFilter
   * @return An appropriately sized set or null if no BloomFiltering required
   */
public:
  virtual std::shared_ptr<FuzzySet>
  getSetForField(std::shared_ptr<SegmentWriteState> state,
                 std::shared_ptr<FieldInfo> info) = 0;

  /**
   * Called when downsizing bitsets for serialization
   *
   * @param fieldInfo
   *          The field with sparse set bits
   * @param initialSet
   *          The bits accumulated
   * @return null or a hopefully more densely packed, smaller bitset
   */
  virtual std::shared_ptr<FuzzySet>
  downsize(std::shared_ptr<FieldInfo> fieldInfo,
           std::shared_ptr<FuzzySet> initialSet);

  /**
   * Used to determine if the given filter has reached saturation and should be
   * retired i.e. not saved any more
   * @param bloomFilter The bloomFilter being tested
   * @param fieldInfo The field with which this filter is associated
   * @return true if the set has reached saturation and should be retired
   */
  virtual bool isSaturated(std::shared_ptr<FuzzySet> bloomFilter,
                           std::shared_ptr<FieldInfo> fieldInfo) = 0;
};

} // #include  "core/src/java/org/apache/lucene/codecs/bloom/
