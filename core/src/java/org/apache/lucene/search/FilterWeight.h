#pragma once
#include "stringhelper.h"
#include <memory>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Weight;
}

namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Matches;
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
using Term = org::apache::lucene::index::Term;

/**
 * A {@code FilterWeight} contains another {@code Weight} and implements
 * all abstract methods by calling the contained weight's method.
 *
 * Note that {@code FilterWeight} does not override the non-abstract
 * {@link Weight#bulkScorer(LeafReaderContext)} method and subclasses of
 * {@code FilterWeight} must provide their bulkScorer implementation
GET_CLASS_NAME(es)
 * if required.
 *
 * @lucene.internal
 */
class FilterWeight : public Weight
{
  GET_CLASS_NAME(FilterWeight)

protected:
  const std::shared_ptr<Weight> in_;

  /**
   * Default constructor.
   */
  FilterWeight(std::shared_ptr<Weight> weight);

  /**
   * Alternative constructor.
   * Use this variant only if the <code>weight</code> was not obtained
   * via the {@link Query#createWeight(IndexSearcher, bool, float)}
   * method of the <code>query</code> object.
   */
  FilterWeight(std::shared_ptr<Query> query, std::shared_ptr<Weight> weight);

public:
  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  void extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

  std::shared_ptr<Scorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                   int doc)  override;

protected:
  std::shared_ptr<FilterWeight> shared_from_this()
  {
    return std::static_pointer_cast<FilterWeight>(Weight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search