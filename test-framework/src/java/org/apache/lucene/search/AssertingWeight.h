#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>

// C++ NOTE: Forward class declarations:
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
class Matches;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class ScorerSupplier;
}
namespace org::apache::lucene::search
{
class BulkScorer;
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

class AssertingWeight : public FilterWeight
{
  GET_CLASS_NAME(AssertingWeight)

public:
  const std::shared_ptr<Random> random;
  const bool needsScores;

  AssertingWeight(std::shared_ptr<Random> random, std::shared_ptr<Weight> in_,
                  bool needsScores);

  std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                   int doc)  override;

  std::shared_ptr<Scorer> scorer(
      std::shared_ptr<LeafReaderContext> context)  override;

  std::shared_ptr<ScorerSupplier> scorerSupplier(
      std::shared_ptr<LeafReaderContext> context)  override;

private:
  class ScorerSupplierAnonymousInnerClass : public ScorerSupplier
  {
    GET_CLASS_NAME(ScorerSupplierAnonymousInnerClass)
  private:
    std::shared_ptr<AssertingWeight> outerInstance;

    std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
        inScorerSupplier;

  public:
    ScorerSupplierAnonymousInnerClass(
        std::shared_ptr<AssertingWeight> outerInstance,
        std::shared_ptr<org::apache::lucene::search::ScorerSupplier>
            inScorerSupplier);

  private:
    bool getCalled = false;

  public:
    std::shared_ptr<Scorer> get(int64_t leadCost)  override;

    int64_t cost() override;

  protected:
    std::shared_ptr<ScorerSupplierAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ScorerSupplierAnonymousInnerClass>(
          ScorerSupplier::shared_from_this());
    }
  };

public:
  std::shared_ptr<BulkScorer> bulkScorer(
      std::shared_ptr<LeafReaderContext> context)  override;

protected:
  std::shared_ptr<AssertingWeight> shared_from_this()
  {
    return std::static_pointer_cast<AssertingWeight>(
        FilterWeight::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
