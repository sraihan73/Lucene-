#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"

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
 * A {@link MergePolicy} that only returns forced merges.
 * <p>
 * <b>NOTE</b>: Use this policy if you wish to disallow background merges but
 * wish to run optimize/forceMerge segment merges.
 *
 * @lucene.experimental
 */
class ForceMergePolicy final : public FilterMergePolicy
{
  GET_CLASS_NAME(ForceMergePolicy)

  /** Create a new {@code ForceMergePolicy} around the given {@code MergePolicy}
   */
public:
  ForceMergePolicy(std::shared_ptr<MergePolicy> in_);

  std::shared_ptr<MergeSpecification> findMerges(
      MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

protected:
  std::shared_ptr<ForceMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<ForceMergePolicy>(
        FilterMergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
