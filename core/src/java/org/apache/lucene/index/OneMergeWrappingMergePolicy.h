#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"

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
 * A wrapping merge policy that wraps the {@link
 * org.apache.lucene.index.MergePolicy.OneMerge} objects returned by the wrapped
 * merge policy.
 *
 * @lucene.experimental
 */
class OneMergeWrappingMergePolicy : public FilterMergePolicy
{
  GET_CLASS_NAME(OneMergeWrappingMergePolicy)

private:
  const std::function<OneMerge *(OneMerge *)> wrapOneMerge;

  /**
   * Constructor
   *
   * @param in - the wrapped merge policy
   * @param wrapOneMerge - operator for wrapping OneMerge objects
   */
public:
  OneMergeWrappingMergePolicy(
      std::shared_ptr<MergePolicy> in_,
      std::function<OneMerge *(OneMerge *)> &wrapOneMerge);

  std::shared_ptr<MergeSpecification> findMerges(
      MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

  std::shared_ptr<MergeSpecification> findForcedMerges(
      std::shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
          &segmentsToMerge,
      std::shared_ptr<MergeContext> mergeContext)  override;

  std::shared_ptr<MergeSpecification> findForcedDeletesMerges(
      std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

private:
  std::shared_ptr<MergeSpecification>
  wrapSpec(std::shared_ptr<MergeSpecification> spec);

protected:
  std::shared_ptr<OneMergeWrappingMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<OneMergeWrappingMergePolicy>(
        FilterMergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
