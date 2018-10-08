#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class MergePolicy;
}

namespace org::apache::lucene::index
{
class SegmentInfos;
}
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
}
namespace org::apache::lucene::index
{
class CodecReader;
}
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
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
namespace org::apache::lucene::index
{

using IOSupplier = org::apache::lucene::util::IOSupplier;

/**
 * A wrapper for {@link MergePolicy} instances.
 *
 * @lucene.experimental
 */
class FilterMergePolicy : public MergePolicy
{
  GET_CLASS_NAME(FilterMergePolicy)

  /** The wrapped {@link MergePolicy}. */
protected:
  const std::shared_ptr<MergePolicy> in_;

  /**
   * Creates a new filter merge policy instance wrapping another.
   *
   * @param in the wrapped {@link MergePolicy}
   */
public:
  FilterMergePolicy(std::shared_ptr<MergePolicy> in_);

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

  bool useCompoundFile(
      std::shared_ptr<SegmentInfos> infos,
      std::shared_ptr<SegmentCommitInfo> mergedInfo,
      std::shared_ptr<MergeContext> mergeContext)  override;

protected:
  int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> context)  override;

public:
  double getNoCFSRatio() override;

  void setNoCFSRatio(double noCFSRatio) override final;

  void setMaxCFSSegmentSizeMB(double v) override final;

  double getMaxCFSSegmentSizeMB() override final;

  virtual std::wstring toString();

  bool
  keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                              readerIOSupplier)  override;

  int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info, int delCount,
                        IOSupplier<std::shared_ptr<CodecReader>>
                            readerSupplier)  override;

protected:
  std::shared_ptr<FilterMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<FilterMergePolicy>(
        MergePolicy::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
