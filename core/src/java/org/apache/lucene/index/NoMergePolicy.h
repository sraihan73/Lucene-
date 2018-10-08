#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/SegmentInfos.h"
#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
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
 * A {@link MergePolicy} which never returns merges to execute. Use it if you
 * want to prevent segment merges.
 */
class NoMergePolicy final : public MergePolicy
{
  GET_CLASS_NAME(NoMergePolicy)

  /** Singleton instance. */
public:
  static const std::shared_ptr<MergePolicy> INSTANCE;

private:
  NoMergePolicy();

public:
  std::shared_ptr<MergeSpecification>
  findMerges(MergeTrigger mergeTrigger,
             std::shared_ptr<SegmentInfos> segmentInfos,
             std::shared_ptr<MergeContext> mergeContext) override;
  std::shared_ptr<MergeSpecification>
  findForcedMerges(std::shared_ptr<SegmentInfos> segmentInfos,
                   int maxSegmentCount,
                   std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
                       &segmentsToMerge,
                   std::shared_ptr<MergeContext> mergeContext) override;
  std::shared_ptr<MergeSpecification>
  findForcedDeletesMerges(std::shared_ptr<SegmentInfos> segmentInfos,
                          std::shared_ptr<MergeContext> mergeContext) override;
  bool useCompoundFile(std::shared_ptr<SegmentInfos> segments,
                       std::shared_ptr<SegmentCommitInfo> newSegment,
                       std::shared_ptr<MergeContext> mergeContext) override;

protected:
  int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> context)  override;

public:
  double getNoCFSRatio() override;

  double getMaxCFSSegmentSizeMB() override;

  void setMaxCFSSegmentSizeMB(double v) override;

  void setNoCFSRatio(double noCFSRatio) override;

  bool
  keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                              readerIOSupplier)  override;

  int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info, int delCount,
                        IOSupplier<std::shared_ptr<CodecReader>>
                            readerSupplier)  override;

  virtual std::wstring toString();

protected:
  std::shared_ptr<NoMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<NoMergePolicy>(
        MergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
