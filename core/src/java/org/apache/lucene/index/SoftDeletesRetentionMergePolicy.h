#pragma once
#include "stringhelper.h"
#include <functional>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/index/MergePolicy.h"
#include  "core/src/java/org/apache/lucene/index/CodecReader.h"
namespace org::apache::lucene::util
{
template <typename T>
class IOSupplier;
}
#include  "core/src/java/org/apache/lucene/util/Bits.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
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

using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Bits = org::apache::lucene::util::Bits;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;
using IOSupplier = org::apache::lucene::util::IOSupplier;

/**
 * This {@link MergePolicy} allows to carry over soft deleted documents across
 * merges. The policy wraps the merge reader and marks documents as "live" that
 * have a value in the soft delete field and match the provided query. This
 * allows for instance to keep documents alive based on time or any other
 * constraint in the index. The main purpose for this merge policy is to
 * implement retention policies for document modification to vanish in the
 * index. Using this merge policy allows to control when soft deletes are
 * claimed by merges.
 * @lucene.experimental
 */
class SoftDeletesRetentionMergePolicy final : public OneMergeWrappingMergePolicy
{
  GET_CLASS_NAME(SoftDeletesRetentionMergePolicy)
private:
  const std::wstring field;
  const std::function<Query *()> retentionQuerySupplier;
  /**
   * Creates a new {@link SoftDeletesRetentionMergePolicy}
   * @param field the soft deletes field
   * @param retentionQuerySupplier a query supplier for the retention query
   * @param in the wrapped MergePolicy
   */
public:
  SoftDeletesRetentionMergePolicy(
      const std::wstring &field,
      std::function<Query *()> &retentionQuerySupplier,
      std::shared_ptr<MergePolicy> in_);

  bool
  keepFullyDeletedSegment(IOSupplier<std::shared_ptr<CodecReader>>
                              readerIOSupplier)  override;

  // pkg private for testing
  static std::shared_ptr<CodecReader>
  applyRetentionQuery(const std::wstring &softDeleteField,
                      std::shared_ptr<Query> retentionQuery,
                      std::shared_ptr<CodecReader> reader) ;

private:
  class BitsAnonymousInnerClass
      : public std::enable_shared_from_this<BitsAnonymousInnerClass>,
        public Bits
  {
    GET_CLASS_NAME(BitsAnonymousInnerClass)
  private:
    std::shared_ptr<Bits> liveDocs;

  public:
    BitsAnonymousInnerClass(std::shared_ptr<Bits> liveDocs);

    bool get(int index) override;

    int length() override;
  };

  /**
   * Clones the given live docs
   */
public:
  static std::shared_ptr<FixedBitSet>
  cloneLiveDocs(std::shared_ptr<Bits> liveDocs);

private:
  static std::shared_ptr<Scorer>
  getScorer(std::shared_ptr<Query> query,
            std::shared_ptr<CodecReader> reader) ;

  /**
   * Returns a codec reader with the given live docs
   */
  static std::shared_ptr<CodecReader>
  wrapLiveDocs(std::shared_ptr<CodecReader> reader,
               std::shared_ptr<Bits> liveDocs, int numDocs);

private:
  class FilterCodecReaderAnonymousInnerClass : public FilterCodecReader
  {
    GET_CLASS_NAME(FilterCodecReaderAnonymousInnerClass)
  private:
    std::shared_ptr<org::apache::lucene::index::CodecReader> reader;
    std::shared_ptr<Bits> liveDocs;
    int numDocs = 0;

  public:
    FilterCodecReaderAnonymousInnerClass(
        std::shared_ptr<org::apache::lucene::index::CodecReader> reader,
        std::shared_ptr<Bits> liveDocs, int numDocs);

    std::shared_ptr<CacheHelper> getCoreCacheHelper() override;

    std::shared_ptr<CacheHelper> getReaderCacheHelper() override;

    std::shared_ptr<Bits> getLiveDocs() override;

    int numDocs() override;

  protected:
    std::shared_ptr<FilterCodecReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCodecReaderAnonymousInnerClass>(
          FilterCodecReader::shared_from_this());
    }
  };

public:
  int numDeletesToMerge(std::shared_ptr<SegmentCommitInfo> info, int delCount,
                        IOSupplier<std::shared_ptr<CodecReader>>
                            readerSupplier)  override;

protected:
  std::shared_ptr<SoftDeletesRetentionMergePolicy> shared_from_this()
  {
    return std::static_pointer_cast<SoftDeletesRetentionMergePolicy>(
        OneMergeWrappingMergePolicy::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
