#pragma once
#include "stringhelper.h"
#include <iostream>
#include <memory>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
#include  "core/src/java/org/apache/lucene/index/MergeReaderWrapper.h"

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

/**
 * MergePolicy that makes random decisions for testing.
 */
class MockRandomMergePolicy : public MergePolicy
{
  GET_CLASS_NAME(MockRandomMergePolicy)
private:
  const std::shared_ptr<Random> random;

public:
  bool doNonBulkMerges = true;

  MockRandomMergePolicy(std::shared_ptr<Random> random);

  /**
   * Set to true if sometimes readers to be merged should be wrapped in a
   * FilterReader to mixup bulk merging.
   */
  virtual void setDoNonBulkMerges(bool v);

  std::shared_ptr<MergeSpecification>
  findMerges(MergeTrigger mergeTrigger,
             std::shared_ptr<SegmentInfos> segmentInfos,
             std::shared_ptr<MergeContext> mergeContext) override;

  std::shared_ptr<MergeSpecification> findForcedMerges(
      std::shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
          &segmentsToMerge,
      std::shared_ptr<MergeContext> mergeContext)  override;

  std::shared_ptr<MergeSpecification> findForcedDeletesMerges(
      std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

  bool useCompoundFile(
      std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<SegmentCommitInfo> mergedInfo,
      std::shared_ptr<MergeContext> mergeContext)  override;

public:
  class MockRandomOneMerge : public OneMerge
  {
    GET_CLASS_NAME(MockRandomOneMerge)
  public:
    const std::shared_ptr<Random> r;

    MockRandomOneMerge(
        std::deque<std::shared_ptr<SegmentCommitInfo>> &segments,
        int64_t seed);

    std::shared_ptr<CodecReader> wrapForMerge(
        std::shared_ptr<CodecReader> reader)  override;

  private:
    class FilterLeafReaderAnonymousInnerClass : public FilterLeafReader
    {
      GET_CLASS_NAME(FilterLeafReaderAnonymousInnerClass)
    private:
      std::shared_ptr<MockRandomOneMerge> outerInstance;

    public:
      FilterLeafReaderAnonymousInnerClass(
          std::shared_ptr<MockRandomOneMerge> outerInstance,
          std::shared_ptr<MergeReaderWrapper> new) new;

      std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

      std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    protected:
      std::shared_ptr<FilterLeafReaderAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<FilterLeafReaderAnonymousInnerClass>(
            FilterLeafReader::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<MockRandomOneMerge> shared_from_this()
    {
      return std::static_pointer_cast<MockRandomOneMerge>(
          OneMerge::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MockRandomMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<MockRandomMergePolicy>(
        MergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
