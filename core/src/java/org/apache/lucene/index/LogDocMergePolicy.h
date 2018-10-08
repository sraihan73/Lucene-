#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class SegmentCommitInfo;
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

/** This is a {@link LogMergePolicy} that measures size of a
 *  segment as the number of documents (not taking deletions
 *  into account). */

class LogDocMergePolicy : public LogMergePolicy
{
  GET_CLASS_NAME(LogDocMergePolicy)

  /** Default minimum segment size.  @see setMinMergeDocs */
public:
  static constexpr int DEFAULT_MIN_MERGE_DOCS = 1000;

  /** Sole constructor, setting all settings to their
   *  defaults. */
  LogDocMergePolicy();

protected:
  int64_t
  size(std::shared_ptr<SegmentCommitInfo> info,
       std::shared_ptr<MergeContext> mergeContext)  override;

  /** Sets the minimum size for the lowest level segments.
   * Any segments below this size are considered to be on
   * the same level (even if they vary drastically in size)
   * and will be merged whenever there are mergeFactor of
   * them.  This effectively truncates the "long tail" of
   * small segments that would otherwise be created into a
   * single level.  If you set this too large, it could
   * greatly increase the merging cost during indexing (if
   * you flush many small segments). */
public:
  virtual void setMinMergeDocs(int minMergeDocs);

  /** Get the minimum size for a segment to remain
   *  un-merged.
   *  @see #setMinMergeDocs **/
  virtual int getMinMergeDocs();

protected:
  std::shared_ptr<LogDocMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<LogDocMergePolicy>(
        LogMergePolicy::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
