#pragma once
#include "stringhelper.h"
#include <memory>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search::spans
{
class SpanWeight;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search::spans
{
class Spans;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search::spans
{
class SpanScorer;
}
namespace org::apache::lucene::search
{
class Explanation;
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

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Similarity = org::apache::lucene::search::similarities::Similarity;

/**
 * Wraps a SpanWeight with additional asserts
 */
class AssertingSpanWeight : public SpanWeight
{
  GET_CLASS_NAME(AssertingSpanWeight)

public:
  const std::shared_ptr<SpanWeight> in_;

  /**
   * Create an AssertingSpanWeight
   * @param in the SpanWeight to wrap
   * @throws IOException on error
   */
  AssertingSpanWeight(std::shared_ptr<IndexSearcher> searcher,
                      std::shared_ptr<SpanWeight> in_) ;

  void extractTermContexts(
      std::unordered_map<std::shared_ptr<Term>, std::shared_ptr<TermContext>>
          &contexts) override;

  std::shared_ptr<Spans>
  getSpans(std::shared_ptr<LeafReaderContext> context,
           Postings requiredPostings)  override;

  std::shared_ptr<Similarity::SimScorer> getSimScorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  void extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

  std::shared_ptr<SpanScorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  std::shared_ptr<Explanation>
  explain(std::shared_ptr<LeafReaderContext> context,
          int doc)  override;

protected:
  std::shared_ptr<AssertingSpanWeight> shared_from_this()
  {
    return std::static_pointer_cast<AssertingSpanWeight>(
        SpanWeight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::spans
