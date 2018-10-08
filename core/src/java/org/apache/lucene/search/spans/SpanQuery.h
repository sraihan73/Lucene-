#pragma once
#include "stringhelper.h"
#include <map_obj>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class IndexSearcher;
}

namespace org::apache::lucene::search::spans
{
class SpanWeight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class TermContext;
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
namespace org::apache::lucene::search::spans
{

using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;

/** Base class for span-based queries. */
class SpanQuery : public Query
{

  /**
   * Returns the name of the field matched by this query.
   */
public:
  virtual std::wstring getField() = 0;

  std::shared_ptr<SpanWeight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost) = 0;
  override override;

  /**
   * Build a map_obj of terms to termcontexts, for use in constructing SpanWeights
   * @lucene.internal
   */
  static std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
  getTermContexts(std::deque<SpanWeight> &weights);

  /**
   * Build a map_obj of terms to termcontexts, for use in constructing SpanWeights
   * @lucene.internal
   */
  static std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
  getTermContexts(
      std::shared_ptr<std::deque<std::shared_ptr<SpanWeight>>> weights);

protected:
  std::shared_ptr<SpanQuery> shared_from_this()
  {
    return std::static_pointer_cast<SpanQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
