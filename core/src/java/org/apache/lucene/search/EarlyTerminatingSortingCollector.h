#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Sort;
}

namespace org::apache::lucene::search
{
class Collector;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class LeafCollector;
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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Collector = org::apache::lucene::search::Collector;
using FilterCollector = org::apache::lucene::search::FilterCollector;
using LeafCollector = org::apache::lucene::search::LeafCollector;
using Sort = org::apache::lucene::search::Sort;

/**
 * A {@link Collector} that early terminates collection of documents on a
 * per-segment basis, if the segment was sorted according to the given
 * {@link Sort}.
 *
 * <p>
 * <b>NOTE:</b> the {@code Collector} detects segments sorted according to a
 * an {@link IndexWriterConfig#setIndexSort}. Also, it collects up to a
 * specified
 * {@code numDocsToCollect} from each segment, and therefore is mostly suitable
 * for use in conjunction with collectors such as {@link TopDocsCollector}, and
 * not e.g. {@link TotalHitCountCollector}.
 * <p>
 * <b>NOTE</b>: If you wrap a {@code TopDocsCollector} that sorts in the same
 * order as the index order, the returned {@link TopDocsCollector#topDocs()
 * TopDocs} will be correct. However the total of {@link
 * TopDocsCollector#getTotalHits() hit count} will be vastly underestimated
 * since not all matching documents will have been collected.
 *
 * @deprecated Pass trackTotalHits=false to {@link TopFieldCollector} instead of
 * using this class.
 */
// C++ TODO: Most Java annotations will not have direct C++ equivalents:
// ORIGINAL LINE: @Deprecated public class EarlyTerminatingSortingCollector
// extends org.apache.lucene.search.FilterCollector
class EarlyTerminatingSortingCollector : public FilterCollector
{

  /** Returns whether collection can be early-terminated if it sorts with the
   *  provided {@link Sort} and if segments are merged with the provided
   *  {@link Sort}. */
public:
  static bool canEarlyTerminate(std::shared_ptr<Sort> searchSort,
                                std::shared_ptr<Sort> mergePolicySort);

  /** Sort used to sort the search results */
protected:
  const std::shared_ptr<Sort> sort;
  /** Number of documents to collect in each segment */
  const int numDocsToCollect;

private:
  // C++ NOTE: Fields cannot have the same name as methods:
  const std::shared_ptr<AtomicBoolean> terminatedEarly_ =
      std::make_shared<AtomicBoolean>(false);

  /**
   * Create a new {@link EarlyTerminatingSortingCollector} instance.
   *
   * @param in
   *          the collector to wrap
   * @param sort
   *          the sort you are sorting the search results on
   * @param numDocsToCollect
   *          the number of documents to collect on each segment. When wrapping
   *          a {@link TopDocsCollector}, this number should be the number of
   *          hits.
   * @throws IllegalArgumentException if the sort order doesn't allow for early
   *          termination with the given merge policy.
   */
public:
  EarlyTerminatingSortingCollector(std::shared_ptr<Collector> in_,
                                   std::shared_ptr<Sort> sort,
                                   int numDocsToCollect);

  std::shared_ptr<LeafCollector> getLeafCollector(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class FilterLeafCollectorAnonymousInnerClass : public FilterLeafCollector
  {
    GET_CLASS_NAME(FilterLeafCollectorAnonymousInnerClass)
  private:
    std::shared_ptr<EarlyTerminatingSortingCollector> outerInstance;

  public:
    FilterLeafCollectorAnonymousInnerClass(
        std::shared_ptr<EarlyTerminatingSortingCollector> outerInstance,
        std::shared_ptr<LeafCollector> getLeafCollector);

  private:
    int numCollected = 0;

  public:
    void collect(int doc)  override;

  protected:
    std::shared_ptr<FilterLeafCollectorAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterLeafCollectorAnonymousInnerClass>(
          org.apache.lucene.search.FilterLeafCollector::shared_from_this());
    }
  };

public:
  virtual bool terminatedEarly();

protected:
  std::shared_ptr<EarlyTerminatingSortingCollector> shared_from_this()
  {
    return std::static_pointer_cast<EarlyTerminatingSortingCollector>(
        org.apache.lucene.search.FilterCollector::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
