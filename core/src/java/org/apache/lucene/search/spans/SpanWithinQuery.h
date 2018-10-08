#pragma once
#include "stringhelper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/spans/SpanQuery.h"

#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/spans/SpanWeight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/TermContext.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/spans/Spans.h"

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
namespace org::apache::lucene::search::spans
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;

/** Keep matches that are contained within another Spans. */
class SpanWithinQuery final : public SpanContainQuery
{
  GET_CLASS_NAME(SpanWithinQuery)

  /** Construct a SpanWithinQuery matching spans from <code>little</code>
   * that are inside of <code>big</code>.
   * This query has the boost of <code>little</code>.
   * <code>big</code> and <code>little</code> must be in the same field.
   */
public:
  SpanWithinQuery(std::shared_ptr<SpanQuery> big,
                  std::shared_ptr<SpanQuery> little);

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override;

public:
  class SpanWithinWeight : public SpanContainWeight
  {
    GET_CLASS_NAME(SpanWithinWeight)
  private:
    std::shared_ptr<SpanWithinQuery> outerInstance;

  public:
    SpanWithinWeight(std::shared_ptr<SpanWithinQuery> outerInstance,
                     std::shared_ptr<IndexSearcher> searcher,
                     std::unordered_map<std::shared_ptr<Term>,
                                        std::shared_ptr<TermContext>> &terms,
                     std::shared_ptr<SpanWeight> bigWeight,
                     std::shared_ptr<SpanWeight> littleWeight,
                     float boost) ;

    /**
     * Return spans from <code>little</code> that are contained in a spans from
     * <code>big</code>. The payload is from the spans of <code>little</code>.
     */
    std::shared_ptr<Spans>
    getSpans(std::shared_ptr<LeafReaderContext> context,
             Postings requiredPostings)  override;

  private:
    class ContainSpansAnonymousInnerClass : public ContainSpans
    {
      GET_CLASS_NAME(ContainSpansAnonymousInnerClass)
    private:
      std::shared_ptr<SpanWithinWeight> outerInstance;

    public:
      ContainSpansAnonymousInnerClass(
          std::shared_ptr<SpanWithinWeight> outerInstance,
          std::shared_ptr<org::apache::lucene::search::spans::Spans> big,
          std::shared_ptr<org::apache::lucene::search::spans::Spans> little,
          std::shared_ptr<org::apache::lucene::search::spans::Spans> little);

      bool twoPhaseCurrentDocMatches()  override;

      int nextStartPosition()  override;

    protected:
      std::shared_ptr<ContainSpansAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ContainSpansAnonymousInnerClass>(
            ContainSpans::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SpanWithinWeight> shared_from_this()
    {
      return std::static_pointer_cast<SpanWithinWeight>(
          SpanContainWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SpanWithinQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanWithinQuery>(
        SpanContainQuery::shared_from_this());
  }
};
} // #include  "core/src/java/org/apache/lucene/search/spans/
