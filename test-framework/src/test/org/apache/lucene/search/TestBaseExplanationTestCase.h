#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class BulkScorer;
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
namespace org::apache::lucene::search
{

using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;

/**
 * Tests that the {@link BaseExplanationTestCase} helper code, as well as
 * {@link CheckHits#checkNoMatchExplanations} are checking what they are suppose
 * to.
 */
class TestBaseExplanationTestCase : public BaseExplanationTestCase
{
  GET_CLASS_NAME(TestBaseExplanationTestCase)

public:
  virtual void testQueryNoMatchWhenExpected() ;
  virtual void testQueryMatchWhenNotExpected() ;

  virtual void testIncorrectExplainScores() ;

  virtual void testIncorrectExplainMatches() ;

public:
  class BrokenExplainTermQuery final : public TermQuery
  {
    GET_CLASS_NAME(BrokenExplainTermQuery)
  public:
    const bool toggleExplainMatch;
    const bool breakExplainScores;
    BrokenExplainTermQuery(std::shared_ptr<Term> t, bool toggleExplainMatch,
                           bool breakExplainScores);
    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  protected:
    std::shared_ptr<BrokenExplainTermQuery> shared_from_this()
    {
      return std::static_pointer_cast<BrokenExplainTermQuery>(
          TermQuery::shared_from_this());
    }
  };

public:
  class BrokenExplainWeight final : public FilterWeight
  {
    GET_CLASS_NAME(BrokenExplainWeight)
  public:
    BrokenExplainWeight(std::shared_ptr<BrokenExplainTermQuery> q,
                        std::shared_ptr<Weight> in_);
    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<BrokenExplainWeight> shared_from_this()
    {
      return std::static_pointer_cast<BrokenExplainWeight>(
          FilterWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestBaseExplanationTestCase> shared_from_this()
  {
    return std::static_pointer_cast<TestBaseExplanationTestCase>(
        BaseExplanationTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
