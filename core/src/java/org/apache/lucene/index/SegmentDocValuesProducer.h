#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"

#include  "core/src/java/org/apache/lucene/index/FieldInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/SegmentDocValues.h"
#include  "core/src/java/org/apache/lucene/store/Directory.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
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
namespace org::apache::lucene::index
{

using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using Directory = org::apache::lucene::store::Directory;
using Accountable = org::apache::lucene::util::Accountable;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/** Encapsulates multiple producers when there are docvalues updates as one
 * producer */
// TODO: try to clean up close? no-op?
// TODO: add shared base class (also used by per-field-pf?) to allow "punching
// thru" to low level producer?
class SegmentDocValuesProducer : public DocValuesProducer
{

private:
  static const int64_t LONG_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(Long::typeid);
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(
          SegmentDocValuesProducer::typeid);

public:
  const std::unordered_map<std::wstring, std::shared_ptr<DocValuesProducer>>
      dvProducersByField =
          std::unordered_map<std::wstring,
                             std::shared_ptr<DocValuesProducer>>();
  const std::shared_ptr<Set<std::shared_ptr<DocValuesProducer>>> dvProducers =
      Collections::newSetFromMap(
          std::make_shared<
              IdentityHashMap<std::shared_ptr<DocValuesProducer>, bool>>());
  const std::deque<int64_t> dvGens = std::deque<int64_t>();

  /**
   * Creates a new producer that handles updated docvalues fields
   * @param si commit point
   * @param dir directory
   * @param coreInfos fieldinfos for the segment
   * @param allInfos all fieldinfos including updated ones
   * @param segDocValues producer map_obj
   */
  SegmentDocValuesProducer(
      std::shared_ptr<SegmentCommitInfo> si, std::shared_ptr<Directory> dir,
      std::shared_ptr<FieldInfos> coreInfos,
      std::shared_ptr<FieldInfos> allInfos,
      std::shared_ptr<SegmentDocValues> segDocValues) ;

  std::shared_ptr<NumericDocValues>
  getNumeric(std::shared_ptr<FieldInfo> field)  override;

  std::shared_ptr<BinaryDocValues>
  getBinary(std::shared_ptr<FieldInfo> field)  override;

  std::shared_ptr<SortedDocValues>
  getSorted(std::shared_ptr<FieldInfo> field)  override;

  std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
      std::shared_ptr<FieldInfo> field)  override;

  std::shared_ptr<SortedSetDocValues>
  getSortedSet(std::shared_ptr<FieldInfo> field)  override;

  void checkIntegrity()  override;

  virtual ~SegmentDocValuesProducer();

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<SegmentDocValuesProducer> shared_from_this()
  {
    return std::static_pointer_cast<SegmentDocValuesProducer>(
        org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
