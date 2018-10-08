#pragma once
#include "stringhelper.h"
#include <memory>
#include <optional>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"

#include  "core/src/java/org/apache/lucene/index/SegmentCommitInfo.h"
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

/** This {@link MergePolicy} is used for upgrading all existing segments of
 * an index when calling {@link IndexWriter#forceMerge(int)}.
 * All other methods delegate to the base {@code MergePolicy} given to the
 * constructor. This allows for an as-cheap-as possible upgrade of an older
 * index by only upgrading segments that are created by previous Lucene
 * versions. forceMerge does no longer really merge; it is just used to
 * &quot;forceMerge&quot; older segment versions away. <p>In general one would
 * use {@link IndexUpgrader}, but for a fully customizeable upgrade, you can use
 * this like any other {@code MergePolicy} and call {@link
 * IndexWriter#forceMerge(int)}: <pre class="prettyprint lang-java">
 *  IndexWriterConfig iwc = new IndexWriterConfig(Version.LUCENE_XX, new
 * KeywordAnalyzer()); iwc.setMergePolicy(new
 * UpgradeIndexMergePolicy(iwc.getMergePolicy())); IndexWriter w = new
 * IndexWriter(dir, iwc); w.forceMerge(1); w.close();
 * </pre>
 * <p><b>Warning:</b> This merge policy may reorder documents if the index was
 * partially upgraded before calling forceMerge (e.g., documents were added). If
 * your application relies on &quot;monotonicity&quot; of doc IDs (which means
 * that the order in which the documents were added to the index is preserved),
 * do a forceMerge(1) instead. Please note, the delegate {@code MergePolicy} may
 * also reorder documents.
 * @lucene.experimental
 * @see IndexUpgrader
 */
class UpgradeIndexMergePolicy : public FilterMergePolicy
{
  GET_CLASS_NAME(UpgradeIndexMergePolicy)

  /** Wrap the given {@link MergePolicy} and intercept forceMerge requests to
   * only upgrade segments written with previous Lucene versions. */
public:
  UpgradeIndexMergePolicy(std::shared_ptr<MergePolicy> in_);

  /** Returns if the given segment should be upgraded. The default
   * implementation will return {@code !Version.LATEST.equals(si.getVersion())},
   * so all segments created with a different version number than this Lucene
   * version will get upgraded.
   */
protected:
  virtual bool shouldUpgradeSegment(std::shared_ptr<SegmentCommitInfo> si);

public:
  std::shared_ptr<MergeSpecification> findMerges(
      MergeTrigger mergeTrigger, std::shared_ptr<SegmentInfos> segmentInfos,
      std::shared_ptr<MergeContext> mergeContext)  override;

  std::shared_ptr<MergeSpecification> findForcedMerges(
      std::shared_ptr<SegmentInfos> segmentInfos, int maxSegmentCount,
      std::unordered_map<std::shared_ptr<SegmentCommitInfo>, bool>
          &segmentsToMerge,
      std::shared_ptr<MergeContext> mergeContext)  override;

protected:
  std::shared_ptr<UpgradeIndexMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<UpgradeIndexMergePolicy>(
        FilterMergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
