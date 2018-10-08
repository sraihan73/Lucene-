#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/Query.h"

#include  "core/src/java/org/apache/lucene/search/LongValuesSource.h"
#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Matches.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

/** A {@link Query} that allows to have a configurable number or required
 *  matches per document. This is typically useful in order to build queries
 *  whose query terms must all appear in documents.
 *  @lucene.experimental
 */
class CoveringQuery final : public Query
{
  GET_CLASS_NAME(CoveringQuery)

private:
  const std::shared_ptr<std::deque<std::shared_ptr<Query>>> queries;
  const std::shared_ptr<LongValuesSource> minimumNumberMatch;
  // C++ NOTE: Fields cannot have the same name as methods:
  const int hashCode_;

  /**
   * Sole constructor.
   * @param queries Sub queries to match.
   * @param minimumNumberMatch Per-document long value that records how many
   * queries should match. Values that are less than 1 are treated like
   * <tt>1</tt>: only documents that have at least one matching clause will be
   * considered matches. Documents that do not have a value for
   * <tt>minimumNumberMatch</tt> do not match.
   */
public:
  CoveringQuery(std::shared_ptr<std::deque<std::shared_ptr<Query>>> queries,
                std::shared_ptr<LongValuesSource> minimumNumberMatch);

  std::wstring toString(const std::wstring &field) override;

  bool equals(std::any obj) override;

private:
  int computeHashCode();

public:
  virtual int hashCode();

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class CoveringWeight : public Weight
  {
    GET_CLASS_NAME(CoveringWeight)

  private:
    const std::shared_ptr<std::deque<std::shared_ptr<Weight>>> weights;
    const std::shared_ptr<LongValuesSource> minimumNumberMatch;

  public:
    CoveringWeight(std::shared_ptr<Query> query,
                   std::shared_ptr<std::deque<std::shared_ptr<Weight>>> weights,
                   std::shared_ptr<LongValuesSource> minimumNumberMatch);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<CoveringWeight> shared_from_this()
    {
      return std::static_pointer_cast<CoveringWeight>(
          Weight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<CoveringQuery> shared_from_this()
  {
    return std::static_pointer_cast<CoveringQuery>(Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
